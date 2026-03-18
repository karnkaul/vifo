#pragma once
#include "vifo/operation.hpp"
#include "vifo/record.hpp"

namespace vifo::path {
struct Transformer {
	[[nodiscard]] auto remove_if_overwrite(fs::path const& destination) const -> Outcome;
	[[nodiscard]] auto transform(fs::path const& source, fs::path const& destination, Operation operation) const -> Record;
	[[nodiscard]] auto undo(std::span<Record const> records) const -> std::vector<Record>;

	bool overwrite{};
};
} // namespace vifo::path
