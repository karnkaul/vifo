#pragma once
#include "vifo/expression.hpp"
#include "vifo/types.hpp"
#include <expected>
#include <string_view>

namespace vifo::detail {
[[nodiscard]] auto to_error(Error::Type type, expression::Token token, std::string_view input, std::string_view msg) -> std::unexpected<Error>;
} // namespace vifo::detail
