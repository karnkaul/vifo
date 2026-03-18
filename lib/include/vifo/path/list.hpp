#pragma once
#include <filesystem>
#include <vector>

namespace vifo::path {
namespace fs = std::filesystem;

struct List {
	// Nested entries should appear before their parents.
	std::vector<fs::path> paths{};
};
} // namespace vifo::path
