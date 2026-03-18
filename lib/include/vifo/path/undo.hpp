#pragma once
#include "vifo/record.hpp"
#include <vector>

namespace vifo::path {
[[nodiscard]] auto undo_successful(std::span<Record const> records) -> std::vector<Record>;
} // namespace vifo::path
