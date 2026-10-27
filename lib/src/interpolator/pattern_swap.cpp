#include "vifo/interpolator/pattern_swap.hpp"
#include "detail/common.hpp"
#include "vifo/environment.hpp"
#include "vifo/expression.hpp"
#include "vifo/util/util.hpp"
#include <algorithm>
#include <array>
#include <ranges>
#include <string_view>

namespace vifo::interpolator {
namespace {
using namespace std::string_view_literals;

using expression::Expression;
using expression::Identifier;
using expression::Substring;
using expression::Token;

constexpr auto keywords_v = std::array{"year"sv};
[[nodiscard]] constexpr auto is_keyword(std::string_view const word) { return std::ranges::find(keywords_v, word) != keywords_v.end(); }

[[nodiscard]] auto format_error(Token const& token, std::string_view const format, std::string_view const msg) {
	return detail::to_error(Error::Type::Format, token, format, msg);
}

[[nodiscard]] auto escape_for_regex(std::string_view const input) {
	auto ret = std::string{};
	for (auto const c : input) {
		switch (c) {
		case '\\':
		case '(':
		case '.':
		case ')': ret.push_back('\\'); break;
		}
		ret.push_back(c);
	}
	return ret;
}

void append_identifier_regex(std::string& out_regex, std::string_view const name) {
	if (name == "year") {
		out_regex.append(util::regex_year_v);
		return;
	}

	out_regex.append(R"((.*))");
}
} // namespace

auto PatternSwap::create(Format const& format) -> Result<PatternSwap> {
	auto ret = PatternSwap{};

	auto input_expression = Expression{};
	auto output_expression = Expression{};
	return util::verify_non_empty(format)
		.and_then([&] { return expression::parse(format.input); })
		.and_then([&](Expression ie) {
			input_expression = std::move(ie);
			return expression::parse(format.output);
		})
		.and_then([&](Expression oe) {
			output_expression = std::move(oe);
			return ret.build_source(format.input, input_expression);
		})
		.and_then([&] { return ret.build_destination(format.output, std::move(output_expression)); })
		.transform([&] { return std::move(ret); });
}

auto PatternSwap::interpolate(std::string_view const input) -> std::string {
	if (!extract_values(input)) { return {}; }
	return m_environment.interpolate(m_destination);
}

auto PatternSwap::build_source(std::string_view const format, Expression const& expression) -> Result<void> {
	auto regex = std::string{};
	auto variable_found = false;
	for (auto const& atom : expression.atoms) {
		if (auto const* substring = std::get_if<Substring>(&atom.value)) {
			regex.append(escape_for_regex(substring->text));
			continue;
		}

		auto const& identifier = std::get<Identifier>(atom.value);
		auto const is_variable = !is_keyword(identifier.name);
		if (is_variable && variable_found) { return format_error(atom.token, format, std::format("too many variables: {}", identifier.name)); }
		variable_found = is_variable;

		if (m_environment.find_symbol(identifier.name)) {
			return format_error(atom.token, format, std::format("duplicate identifier in input expression: '{}'", identifier.name));
		}

		m_environment.symbols.emplace_back(identifier.name);
		append_identifier_regex(regex, identifier.name);
	}

	m_source = util::RegexMatcher{regex};
	return {};
}

auto PatternSwap::build_destination(std::string_view format, Expression expression) -> Result<void> {
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
	auto matches = m_source.get_matches(input);
	if (matches.size() != m_environment.symbols.size()) { return false; }

	for (auto const& [match, symbol] : std::views::zip(matches, m_environment.symbols)) { symbol.value = std::move(match); }

	return true;
}
} // namespace vifo::interpolator
