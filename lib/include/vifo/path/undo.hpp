#pragma once
#include "vifo/transaction.hpp"

namespace vifo::path {
[[nodiscard]] auto undo_successful(std::span<Record const> records) -> Transaction;
} // namespace vifo::path
