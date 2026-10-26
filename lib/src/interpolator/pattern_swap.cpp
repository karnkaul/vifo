#include "vifo/interpolator/pattern_swap.hpp"
#include "detail/common.hpp"
#include "klib/ptr.hpp"
#include "vifo/environment.hpp"
#include "vifo/expression.hpp"
#include "vifo/util/util.hpp"
#include <algorithm>
#include <array>
#include <ranges>
#include <string_view>

namespace vifo::interpolator {
namespace {
using expression::Expression;
using expression::Identifier;
using expression::Substring;
using expression::Token;

class Binding : public Symbol {
  public:
	explicit Binding(std::string name) : Symbol(std::move(name)) {}

	virtual auto parse_front(std::string_view& out_input) -> bool = 0;
	virtual auto parse_back(std::string_view& out_input) -> bool = 0;
};

class Variable : public Binding {
  public:
	explicit Variable(std::string name) : Binding(std::move(name)) {}

  protected:
	auto parse_front(std::string_view& out_text) -> bool override {
		value = out_text;
		out_text = {};
		return !value.empty();
	}

	auto parse_back(std::string_view& out_text) -> bool override {
		value = out_text;
		out_text = {};
		return !value.empty();
	}
};

class Year : public Binding {
  public:
	static constexpr std::string_view name_v{"year"};

	explicit Year() : Binding(std::string{name_v}) {}

  private:
	auto parse_front(std::string_view& out_input) -> bool final {
		if (out_input.size() < 4) { return false; }
		auto const substr = out_input.substr(0, 4);
		if (!util::is_year(substr)) { return false; }
		value = substr;
		out_input.remove_prefix(4);
		return true;
	}

	auto parse_back(std::string_view& out_input) -> bool final {
		if (out_input.size() < 4) { return false; }
		auto const substr = out_input.substr(out_input.size() - 4);
		if (!util::is_year(substr)) { return false; }
		value = substr;
		out_input.remove_suffix(4);
		return true;
	}
};

constexpr auto keywords_v = std::array{Year::name_v};
[[nodiscard]] constexpr auto is_keyword(std::string_view const word) { return std::ranges::find(keywords_v, word) != keywords_v.end(); }

[[nodiscard]] auto format_error(Token const& token, std::string_view const format, std::string_view const msg) {
	return detail::to_error(Error::Type::Format, token, format, msg);
}

class BindingBuilder {
  public:
	explicit BindingBuilder(Environment& environment, std::string_view const source_format) : m_environment(environment), m_source_format(source_format) {}

	[[nodiscard]] auto create(Token const& token, Identifier const& identifier) -> Result<void> {
		return verify_unique(token, identifier).and_then([&] {
			return create_binding(identifier).transform([&](std::unique_ptr<Binding> binding) { m_environment.symbols.push_back(std::move(binding)); });
		});
	}

  private:
	[[nodiscard]] auto verify_unique(Token const& token, Identifier const& identifier) const -> Result<void> {
		if (m_environment.find_symbol(identifier.name)) {
			return format_error(token, m_source_format, std::format("duplicate identifier in input expression: '{}'", identifier.name));
		}
		return {};
	}

	[[nodiscard]] static auto create_binding(Identifier const& identifier) -> Result<std::unique_ptr<Binding>> {
		if (identifier.name == "year") { return std::make_unique<Year>(); }
		return std::make_unique<Variable>(std::string{identifier.name});
	}

	Environment& m_environment;
	std::string_view m_source_format{};
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

auto PatternSwap::build_source(std::string_view const format, expression::Expression expression) -> Result<void> {
	auto* target = &m_source.pre_variable;
	auto binding_builder = BindingBuilder{m_environment, format};

	for (auto& atom : expression.atoms) {
		auto* identifier = std::get_if<expression::Identifier>(&atom.value);
		if (identifier) {
			auto result = binding_builder.create(atom.token, *identifier);
			if (!result) { return std::unexpected{std::move(result.error())}; }

			if (!is_keyword(identifier->name)) {
				if (target == &m_source.post_variable) { return format_error(atom.token, format, std::format("too many variables: {}", identifier->name)); }

				m_source.variable = std::move(*identifier);
				target = &m_source.post_variable;
				continue;
			}
		}

		target->atoms.push_back(std::move(atom));
	}
	return {};
}

auto PatternSwap::build_destination(std::string_view format, expression::Expression expression) -> Result<void> {
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
	return extract_front(input) && extract_back(input) && extract_variable(input) && input.empty();
}

auto PatternSwap::match_symbol(std::string_view& out_input, expression::Atom const& atom, bool front) -> bool {
	if (auto const* substring = std::get_if<Substring>(&atom.value)) {
		return front ? substring->consume_front(out_input) : substring->consume_back(out_input);
	}

	// TODO: consolidate
	// TODO: remove dynamic_cast
	auto const& identifier = std::get<Identifier>(atom.value);
	auto* binding = dynamic_cast<Binding*>(m_environment.find_symbol(identifier.name).get());
	KLIB_ASSERT(binding);
	return front ? binding->parse_front(out_input) : binding->parse_back(out_input);
}

auto PatternSwap::extract_front(std::string_view& out_input) -> bool {
	// TODO: consolidate
	for (auto const& atom : m_source.pre_variable.atoms) {
		if (!match_symbol(out_input, atom, true)) { return false; }
	}
	return true;
}

auto PatternSwap::extract_back(std::string_view& out_input) -> bool {
	// TODO: consolidate
	for (auto const& atom : std::views::reverse(m_source.post_variable.atoms)) {
		if (!match_symbol(out_input, atom, false)) { return false; }
	}
	return true;
}

auto PatternSwap::extract_variable(std::string_view& out_input) -> bool {
	// TODO: consolidate
	auto* binding = dynamic_cast<Binding*>(m_environment.find_symbol(m_source.variable.name).get());
	KLIB_ASSERT(binding);
	return binding->parse_front(out_input);
}
} // namespace vifo::interpolator
