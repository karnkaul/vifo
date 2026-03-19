#pragma once
#include "vifo/types.hpp"
#include <filesystem>

namespace vifo::path {
namespace fs = std::filesystem;

struct Transformer {
	[[nodiscard]] auto remove_if_overwrite(fs::path const& destination) const -> Outcome;
	[[nodiscard]] auto transform(fs::path const& source, fs::path const& destination, Operation operation) const -> Outcome;

	bool overwrite{};
};
} // namespace vifo::path
