#pragma once
#include "vifo/types.hpp"
#include <expected>

namespace vifo {
template <typename Type>
using Result = std::expected<Type, Error>;
} // namespace vifo
