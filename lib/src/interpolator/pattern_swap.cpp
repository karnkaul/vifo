#include "vifo/interpolator/pattern_swap.hpp"
#include "detail/common.hpp"
#include "klib/ptr.hpp"
#include "vifo/environment.hpp"
#include "vifo/expression.hpp"
#include "vifo/util/util.hpp"
#include <algorithm>
#include <regex>
#include <string_view>

namespace vifo::interpolator {
using expression::Expression;
using expression::Identifier;
using expression::Substring;
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

namespace {
[[nodiscard]] auto format_error(Token const& token, std::string_view const format, std::string_view const msg) {
	return detail::to_error(Error::Type::Format, token, format, msg);
}

class BindingBuilder {
  public:
	explicit BindingBuilder(Environment& environment, std::string_view const source_format) : m_environment(environment), m_source_format(source_format) {}

	[[nodiscard]] auto create(Token const& token, Identifier const& identifier) -> Result<void> {
		return verify_unique(token, identifier).and_then([&] {
			return create_binding(token, identifier).transform([&](std::unique_ptr<Binding> binding) { m_environment.symbols.push_back(std::move(binding)); });
		});
	}

  private:
	[[nodiscard]] auto verify_unique(Token const& token, Identifier const& identifier) const -> Result<void> {
		if (m_environment.find_symbol(identifier.name)) {
			return format_error(token, m_source_format, std::format("duplicate identifier in input expression: '{}'", identifier.name));
		}
		return {};
	}

	[[nodiscard]] auto create_binding(Token const& token, Identifier const& identifier) -> Result<std::unique_ptr<Binding>> {
		if (identifier.name == "year") { return std::make_unique<Year>(); }
		if (identifier.name == "title") { return std::make_unique<Title>(); }

		if (identifier.length == 0) {
			if (m_zero_length_variable) {
				return format_error(token, m_source_format, std::format("excess identifier after 0-length parsed: '{}'", identifier.name));
			}
			m_zero_length_variable = true;
		}
		return std::make_unique<Variable>(std::string{identifier.name}, identifier.length);
	}

	Environment& m_environment;
	std::string_view m_source_format{};
	bool m_zero_length_variable{};
};
} // namespace

auto PatternSwap::create(Format const& format) -> Result<PatternSwap> {
	auto ret = PatternSwap{};

	auto input_expression = Expression{};
	auto output_expression = Expression{};
	return expression::parse(format.input)
		.and_then([&](Expression ie) {
			input_expression = std::move(ie);
			return expression::parse(format.output);
		})
		.and_then([&](Expression oe) {
			output_expression = std::move(oe);
			return ret.build_source(format.input, std::move(input_expression));
		})
		.and_then([&] { return ret.build_destination(format.output, std::move(output_expression)); })
		.transform([&] { return std::move(ret); });
}

auto PatternSwap::interpolate(std::string_view const input) -> std::string {
	if (!extract_values(input)) { return {}; }
	return m_environment.interpolate(m_destination);
}

auto PatternSwap::build_source(std::string_view const format, Expression expression) -> Result<void> {
	m_source = std::move(expression);
	auto binding_builder = BindingBuilder{m_environment, format};
	for (auto& atom : m_source.atoms) {
		auto* identifier = std::get_if<Identifier>(&atom.value);
		if (!identifier) { continue; }

		auto result = binding_builder.create(atom.token, *identifier);
		if (!result) { return std::unexpected{std::move(result.error())}; }
	}

	return {};
}

auto PatternSwap::build_destination(std::string_view const format, Expression expression) -> Result<void> {
	for (auto const& atom : expression.atoms) {
		auto const* identifier = std::get_if<Identifier>(&atom.value);
		if (!identifier) { continue; }

		if (!m_environment.find_symbol(identifier->name)) {
			return format_error(atom.token, format, std::format("undefined identifier in output expression: '{}'", identifier->name));
		}
	}

	m_destination = std::move(expression);
	return {};
}

auto PatternSwap::extract_values(std::string_view input) -> bool {
	for (auto const& atom : m_source.atoms) {
		if (!match_symbol(input, atom)) { return false; }
	}
	return true;
}

auto PatternSwap::match_symbol(std::string_view& out_input, expression::Atom const& atom) -> bool {
	if (auto const* substring = std::get_if<Substring>(&atom.value)) { return substring->consume(out_input); }

	auto const& identifier = std::get<Identifier>(atom.value);
	auto* binding = dynamic_cast<Binding*>(m_environment.find_symbol(identifier.name).get());
	KLIB_ASSERT(binding);
	return binding->parse_value(out_input);
}
} // namespace vifo::interpolator
