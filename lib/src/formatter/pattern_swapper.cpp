#include "vifo/formatter/pattern_swapper.hpp"
#include "detail/common.hpp"
#include "klib/ptr.hpp"
#include "vifo/environment.hpp"
#include "vifo/expression.hpp"
#include "vifo/util/util.hpp"
#include <djson/json.hpp>
#include <algorithm>
#include <regex>
#include <string_view>

namespace vifo {
using expression::Expression;
using expression::Identifier;
using expression::Substring;
using expression::Term;
using expression::Token;

namespace {
class Binding : public Symbol {
  public:
	explicit Binding(std::string name) : Symbol(std::move(name)) {}

	virtual auto parse_value(std::string_view& out_input) -> bool = 0;
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

		value = out_text.substr(0, length);
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

		value = out_input.substr(0, 4);
		out_input.remove_prefix(value.size());
		return true;
	}
};

class Title : public Binding {
  public:
	static constexpr std::string_view name_v{"title"};

	explicit Title() : Binding(std::string{name_v}) {}

  private:
	[[nodiscard]] auto parse_value(std::string_view& out_input) -> bool final {
		value = util::trim_identified_title(out_input);
		return !value.empty();
	}
};
} // namespace

struct PatternSwapContext {
	[[nodiscard]] auto find_binding(std::string_view const name) const -> klib::Ptr<Binding> {
		return dynamic_cast<Binding*>(environment->find_symbol(name).get());
	}

	[[nodiscard]] auto get_value(std::string_view const name) const -> std::string_view { return environment->get_value(name); }

	Term source{};
	Term destination{};
	klib::Ptr<Environment> environment{};
};

namespace {
using Context = PatternSwapContext;

[[nodiscard]] auto format_error(Token const& token, std::string_view const format, std::string_view const msg) {
	return detail::to_error(Error::Type::Format, token, format, msg);
}

class BindingBuilder {
  public:
	explicit BindingBuilder(Context& context) : m_context(context) {}

	[[nodiscard]] auto create(Token const& token, Identifier const& identifier) -> Result<void> {
		return verify_unique(token, identifier).and_then([&] {
			return create_binding(token, identifier).transform([&](std::unique_ptr<Binding> binding) {
				m_context.environment->symbols.push_back(std::move(binding));
			});
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
} // namespace

auto PatternSwapFormat::from_file(std::string_view const path) -> std::optional<PatternSwapFormat> {
	auto const result = dj::Json::from_file(path);
	if (!result) { return {}; }

	auto const& json = *result;
	auto ret = PatternSwapFormat{};
	from_json(json["input"], ret.input);
	from_json(json["output"], ret.output);
	if (ret.input.empty() || ret.output.empty()) { return {}; }

	return ret;
}

void PatternSwapper::Deleter::operator()(Context* ptr) const noexcept { std::default_delete<Context>{}(ptr); }

auto PatternSwapper::create(Format format) -> Result<PatternSwapper> {
	auto ret = PatternSwapper{};
	ret.m_context.reset(new Context); // NOLINT(cppcoreguidelines-owning-memory)
	ret.m_context->environment = ret.m_environment.get();
	ret.m_context->source.format = std::move(format.input);
	ret.m_context->destination.format = std::move(format.output);

	auto input_expression = Expression{};
	auto output_expression = Expression{};
	return expression::parse(ret.m_context->source.format)
		.and_then([&](Expression ie) {
			input_expression = std::move(ie);
			return expression::parse(ret.m_context->destination.format);
		})
		.and_then([&](Expression oe) {
			output_expression = std::move(oe);
			return ret.build_source(std::move(input_expression));
		})
		.and_then([&] { return ret.build_destination(std::move(output_expression)); })
		.transform([&] { return std::move(ret); });
}

auto PatternSwapper::format_string(std::string_view const input) -> std::string {
	if (!extract_values(input)) { return {}; }
	return interpolate(m_context->destination.expression);
}

auto PatternSwapper::build_source(Expression expression) -> Result<void> {
	m_context->source.expression = std::move(expression);
	auto binding_builder = BindingBuilder{*m_context};
	for (auto& atom : m_context->source.expression.atoms) {
		auto* identifier = std::get_if<Identifier>(&atom.value);
		if (!identifier) { continue; }

		auto result = binding_builder.create(atom.token, *identifier);
		if (!result) { return std::unexpected{std::move(result.error())}; }
	}

	return {};
}

auto PatternSwapper::build_destination(Expression destination) -> Result<void> {
	for (auto const& atom : destination.atoms) {
		auto const* identifier = std::get_if<Identifier>(&atom.value);
		if (!identifier) { continue; }

		if (!m_context->find_binding(identifier->name)) {
			return format_error(atom.token, m_context->destination.format, std::format("undefined identifier in output expression: '{}'", identifier->name));
		}
	}

	m_context->destination.expression = std::move(destination);
	return {};
}

auto PatternSwapper::extract_values(std::string_view input) -> bool {
	if (!m_context) { return false; }
	for (auto const& atom : m_context->source.expression.atoms) {
		if (!match_symbol(input, atom)) { return false; }
	}
	return true;
}

auto PatternSwapper::match_symbol(std::string_view& out_input, expression::Atom const& atom) -> bool {
	if (auto const* substring = std::get_if<Substring>(&atom.value)) { return substring->consume(out_input); }

	auto const& identifier = std::get<Identifier>(atom.value);
	auto binding = m_context->find_binding(identifier.name);
	KLIB_ASSERT(binding);
	return binding->parse_value(out_input);
}
} // namespace vifo
