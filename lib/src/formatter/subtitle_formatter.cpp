#include "vifo/formatter/subtitle_formatter.hpp"
#include "detail/common.hpp"
#include "vifo/expression.hpp"
#include <format>
#include <string_view>

namespace vifo {
namespace {
using expression::Expression;
using expression::Identifier;

[[nodiscard]] auto verify_identifiers(std::string_view const format, Expression in) -> Result<Expression> {
	for (auto const& atom : in.atoms) {
		auto const* identifier = std::get_if<Identifier>(&atom.value);
		if (!identifier) { continue; }

		if (identifier->name != SubtitleFormatter::number_identifier_v && identifier->name != SubtitleFormatter::title_identifier_v) {
			return detail::to_error(Error::Type::Format, atom.token, format, std::format("unrecognized identifier: '{}'", identifier->name));
		}
	}
	return in;
}
} // namespace

auto SubtitleFormatter::create(Format format) -> Result<SubtitleFormatter> {
	auto ret = SubtitleFormatter{};
	ret.m_format = std::move(format);

	return expression::parse(ret.m_format.primary)
		.and_then([&](Expression in) { return verify_identifiers(ret.m_format.primary, std::move(in)); })
		.and_then([&](Expression verified) {
			ret.m_primary = std::move(verified);
			return expression::parse(ret.m_format.secondary);
		})
		.and_then([&](Expression in) { return verify_identifiers(ret.m_format.secondary, std::move(in)); })
		.transform([&](Expression verified) {
			ret.m_secondary = std::move(verified);
			return std::move(ret);
		});
}

auto SubtitleFormatter::format_string(std::string_view const input) -> std::string {
	if (input.empty()) { return {}; }
	auto ret = [&] {
		if (m_number == 0) { return interpolate(m_primary); }
		return interpolate(m_secondary);
	}();
	set_number(m_number + 1);
	return ret;
}

void SubtitleFormatter::set_number(int const number) {
	m_number = number;
	m_environment->set_symbol(number_identifier_v, std::format("{:02}", m_number));
}

void SubtitleFormatter::set_title(std::string title) { m_environment->set_symbol(title_identifier_v, std::move(title)); }

auto SubtitleFormatter::format_number() -> std::string { return std::format("{:02}", m_number++); }
} // namespace vifo
