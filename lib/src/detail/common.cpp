#include "detail/common.hpp"
#include <format>
#include <string_view>

namespace vifo {
namespace {
[[nodiscard]] auto format_message(Error::Type const type, std::string_view const msg) {
	return std::format("[{}] {}", Error::type_name_map.to_name(type), msg);
}
} // namespace

auto detail::to_error(Error::Type type, expression::Token token, std::string_view input, std::string_view msg) -> std::unexpected<Error> {
	auto ret = Error{.type = type, .message = format_message(type, msg)};
	if (!input.empty()) {
		std::format_to(std::back_inserter(ret.message), "\n | {}\n | ", input);
		for (std::size_t i = 0; i < token.start_index; ++i) { ret.message.push_back(' '); }
		for (std::size_t i = 0; i < token.length; ++i) { ret.message.push_back('^'); }
	}
	return std::unexpected{std::move(ret)};
}

auto detail::to_error(Error::Type type, std::string_view msg) -> std::unexpected<Error> {
	return std::unexpected{Error{.type = type, .message = format_message(type, msg)}};
}
} // namespace vifo
