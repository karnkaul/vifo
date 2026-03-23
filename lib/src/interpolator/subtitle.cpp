#include "vifo/interpolator/subtitle.hpp"
#include "detail/common.hpp"
#include "vifo/expression.hpp"
#include <filesystem>
#include <format>
#include <string_view>

namespace vifo::interpolator {
namespace {
using expression::Expression;
using expression::Identifier;

[[nodiscard]] auto verify_identifiers(std::string_view const format, Expression in) -> Result<Expression> {
	for (auto const& atom : in.atoms) {
		auto const* identifier = std::get_if<Identifier>(&atom.value);
		if (!identifier) { continue; }

		if (identifier->name != Subtitle::number_identifier_v && identifier->name != Subtitle::title_identifier_v) {
			return detail::to_error(Error::Type::Format, atom.token, format, std::format("unrecognized identifier: '{}'", identifier->name));
		}
	}
	return in;
}
} // namespace

auto Subtitle::create(Format const& format) -> Result<Subtitle> {
	auto ret = Subtitle{};

	return expression::parse(format.output)
		.and_then([&](Expression in) { return verify_identifiers(format.output, std::move(in)); })
		.transform([&](Expression verified) {
			ret.m_output = std::move(verified);
			return std::move(ret);
		});
}

void Subtitle::set_number(int const number) {
	m_number = number;
	m_environment.set_symbol(number_identifier_v, std::format("{:02}", m_number));
}

void Subtitle::set_title(std::string title) {
	m_environment.set_symbol(title_identifier_v, std::move(title));
	set_number(0);
}

auto Subtitle::interpolate_stem() -> std::string {
	set_number(m_number + 1);
	return m_environment.interpolate(m_output);
}

auto Subtitle::interpolate_path(fs::path const& parent, std::string_view const extension) -> fs::path {
	auto ret = parent / interpolate_stem();
	ret += extension;
	return ret;
}
} // namespace vifo::interpolator
