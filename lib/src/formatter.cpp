#include "vifo/formatter.hpp"
#include "detail/common.hpp"
#include "djson/json.hpp"
#include "klib/ptr.hpp"
#include "vifo/expression.hpp"
#include "vifo/result.hpp"
#include "vifo/types.hpp"
#include "vifo/util/util.hpp"
#include <algorithm>
#include <cstdlib>
#include <regex>
#include <string_view>

namespace vifo {
using expression::Expression;
using expression::Identifier;
using expression::Substring;
using expression::Token;

namespace {
class Binding : public klib::Polymorphic {
  public:
	explicit Binding(std::string name) : m_name(std::move(name)) {}

	virtual auto parse_value(std::string_view& out_input) -> bool = 0;

	[[nodiscard]] auto get_name() const -> std::string_view { return m_name; }
	[[nodiscard]] auto get_value() const -> std::string_view { return m_value; }

  protected:
	std::string m_value{};

  private:
	std::string m_name{};
};

class Variable : public Binding {
  public:
	explicit Variable(std::string name, std::size_t const max_length) : Binding(std::move(name)), m_max_length(max_length) {}

  private:
	auto parse_value(std::string_view& out_text) -> bool final {
		auto const length = [&] {
			if (m_max_length == 0) { return out_text.length(); }
			return std::min(m_max_length, out_text.length());
		}();

		m_value = out_text.substr(0, length);
		out_text.remove_prefix(length);
		return true;
	}

	std::size_t m_max_length{};
};

class Year : public Binding {
  public:
	static constexpr std::string_view name_v{"year"};

	explicit Year() : Binding(std::string{name_v}) {}

  private:
	[[nodiscard]] auto parse_value(std::string_view& out_input) -> bool final {
		if (out_input.size() < 4) { return false; }

		static auto const s_regex = std::regex{R"([1-9]\d{3}(?!\d).*)"};
		char const* end = out_input.data() + out_input.size();
		if (!std::regex_match(out_input.data(), end, s_regex)) { return false; }

		m_value = out_input.substr(0, 4);
		out_input.remove_prefix(m_value.size());
		return true;
	}
};

class Title : public Binding {
  public:
	static constexpr std::string_view name_v{"title"};

	explicit Title() : Binding(std::string{name_v}) {}

  private:
	[[nodiscard]] auto parse_value(std::string_view& out_input) -> bool final {
		m_value = util::trim_identified_title(out_input);
		return !m_value.empty();
	}
};

struct Term {
	std::string format{};
	Expression expression{};
};

struct Context {
	[[nodiscard]] auto find_binding(std::string_view const name) const -> klib::Ptr<Binding> {
		auto const it = std::ranges::find_if(bindings, [name](auto const& b) { return b->get_name() == name; });
		if (it == bindings.end()) { return nullptr; }
		return it->get();
	}

	Term source{};
	Term transform{};
	std::vector<std::unique_ptr<Binding>> bindings{};
};

[[nodiscard]] auto format_error(Token const& token, std::string_view const format, std::string_view const msg) {
	return detail::to_error(Error::Type::Format, token, format, msg);
}

class BindingBuilder {
  public:
	explicit BindingBuilder(Context& context) : m_context(context) {}

	[[nodiscard]] auto create(Token const& token, Identifier const& identifier) -> Result<void> {
		return verify_unique(token, identifier).and_then([&] {
			return create_binding(token, identifier).transform([&](std::unique_ptr<Binding> binding) { m_context.bindings.push_back(std::move(binding)); });
		});
	}

  private:
	[[nodiscard]] auto verify_unique(Token const& token, Identifier const& identifier) const -> Result<void> {
		if (m_context.find_binding(identifier.name)) {
			return format_error(token, m_context.source.format, std::format("duplicate identifier in input expression: '{}'", identifier.name));
		}
		return {};
	}

	[[nodiscard]] auto create_binding(Token const& token, Identifier const& identifier) -> Result<std::unique_ptr<Binding>> {
		if (identifier.name == "year") { return std::make_unique<Year>(); }
		if (identifier.name == "title") { return std::make_unique<Title>(); }

		if (identifier.length == 0) {
			if (m_zero_length_variable) {
				return format_error(token, m_context.source.format, std::format("excess identifier after 0-length parsed: '{}'", identifier.name));
			}
			m_zero_length_variable = true;
		}
		return std::make_unique<Variable>(std::string{identifier.name}, identifier.length);
	}

	Context& m_context;
	bool m_zero_length_variable{};
};

class Interpolator : public IFormatter {
  public:
	[[nodiscard]] auto initialize(InterpolateFormat format) -> Result<void> {
		m_context.source.format = std::move(format.input);
		m_context.transform.format = std::move(format.output);

		auto input_expression = Expression{};
		auto output_expression = Expression{};
		return expression::parse(m_context.source.format)
			.and_then([&](Expression ie) {
				input_expression = std::move(ie);
				return expression::parse(m_context.transform.format);
			})
			.and_then([&](Expression oe) {
				output_expression = std::move(oe);
				return build_source(std::move(input_expression));
			})
			.and_then([&] { return build_transform(std::move(output_expression)); });
	}

  private:
	[[nodiscard]] auto format(std::string_view const input) -> std::string final {
		if (!extract_values(input)) { return {}; }
		return interpolate();
	}

	[[nodiscard]] auto build_source(Expression expression) -> Result<void> {
		m_context.source.expression = std::move(expression);
		auto binding_builder = BindingBuilder{m_context};
		for (auto& atom : m_context.source.expression.atoms) {
			auto* identifier = std::get_if<Identifier>(&atom.value);
			if (!identifier) { continue; }

			auto result = binding_builder.create(atom.token, *identifier);
			if (!result) { return std::unexpected{std::move(result.error())}; }
		}

		return {};
	}

	[[nodiscard]] auto build_transform(Expression transform) -> Result<void> {
		for (auto const& atom : transform.atoms) {
			auto const* identifier = std::get_if<Identifier>(&atom.value);
			if (!identifier) { continue; }

			if (!m_context.find_binding(identifier->name)) {
				return format_error(atom.token, m_context.transform.format, std::format("undefined identifier in output expression: '{}'", identifier->name));
			}
		}

		m_context.transform.expression = std::move(transform);
		return {};
	}

	[[nodiscard]] auto extract_values(std::string_view input) -> bool {
		for (auto const& atom : m_context.source.expression.atoms) {
			if (!match_symbol(input, atom)) { return false; }
		}
		return true;
	}

	[[nodiscard]] auto match_symbol(std::string_view& out_input, expression::Atom const& atom) -> bool {
		if (auto const* substring = std::get_if<Substring>(&atom.value)) { return substring->consume(out_input); }

		auto const& identifier = std::get<Identifier>(atom.value);
		auto binding = m_context.find_binding(identifier.name);
		KLIB_ASSERT(binding);
		return binding->parse_value(out_input);
	}

	[[nodiscard]] auto interpolate() -> std::string {
		auto ret = std::string{};
		for (auto const& atom : m_context.transform.expression.atoms) {
			if (auto const* substring = std::get_if<Substring>(&atom.value)) {
				ret += substring->text;
				continue;
			}

			auto const& identifer = std::get<Identifier>(atom.value);
			auto const binding = m_context.find_binding(identifer.name);
			if (!binding) { return {}; }
			ret += binding->get_value();
		}

		return ret;
	}

	Context m_context{};
};
} // namespace

auto InterpolateFormat::from_file(std::string_view const path) -> std::optional<InterpolateFormat> {
	auto const result = dj::Json::from_file(path);
	if (!result) { return {}; }

	auto const& json = *result;
	auto ret = InterpolateFormat{};
	from_json(json["input"], ret.input);
	from_json(json["output"], ret.output);
	if (ret.input.empty() || ret.output.empty()) { return {}; }

	return ret;
}
} // namespace vifo

auto vifo::create_interpolator(InterpolateFormat format) -> Result<std::unique_ptr<IFormatter>> {
	auto ret = std::make_unique<Interpolator>();
	return ret->initialize(std::move(format)).transform([&] { return std::move(ret); });
}
