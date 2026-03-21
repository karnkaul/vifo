#include "detail/formatter/subtitle_formatter.hpp"
#include "detail/common.hpp"
#include "vifo/expression.hpp"
#include "vifo/formatter.hpp"
#include <format>
#include <string_view>

namespace vifo::detail {
namespace {
using expression::Expression;
using expression::Identifier;

[[nodiscard]] auto verify_identifiers(std::string_view const format, Expression in) -> Result<Expression> {
	for (auto const& atom : in.atoms) {
		auto const* identifier = std::get_if<Identifier>(&atom.value);
		if (!identifier) { continue; }

		if (identifier->name != ISubtitleFormatter::number_identifier_v && identifier->name != ISubtitleFormatter::title_identifier_v) {
			return detail::to_error(Error::Type::Format, atom.token, format, std::format("unrecognized identifier: '{}'", identifier->name));
		}
	}
	return in;
}
} // namespace

auto SubtitleFormatter::initialize(Format format) -> Result<void> {
	m_format = std::move(format);

	return expression::parse(m_format.primary)
		.and_then([&](Expression in) { return verify_identifiers(m_format.primary, std::move(in)); })
		.and_then([&](Expression verified) {
			m_primary = std::move(verified);
			return expression::parse(m_format.secondary);
		})
		.and_then([&](Expression in) { return verify_identifiers(m_format.secondary, std::move(in)); })
		.transform([&](Expression verified) { m_secondary = std::move(verified); });
}

auto SubtitleFormatter::format_string(std::string_view const input) -> std::string {
	if (input.empty()) { return {}; }
	auto const expression = [&] -> Expression const& {
		if (m_number == 0) { return m_primary; }
		return m_secondary;
	}();

	auto const number = format_number();
	auto const get_value = [&](Identifier const& identifier) -> std::string_view {
		if (identifier.name == title_identifier_v) { return m_title; }
		if (identifier.name == number_identifier_v) { return number; }
		return {};
	};
	return expression.interpolate(get_value);
}

auto SubtitleFormatter::format_number() -> std::string { return std::format("{:02}", m_number++); }
} // namespace vifo::detail
