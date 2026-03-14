#pragma once
#include "vifo/error.hpp"
#include <expected>
#include <string_view>

namespace vifo {
template <typename Type>
using Result = std::expected<Type, Error>;

[[nodiscard]] inline auto to_error(Error::Type const type, std::string_view const msg) -> std::unexpected<Error> {
	return std::unexpected{Error::from_msg(type, msg)};
}
} // namespace vifo
