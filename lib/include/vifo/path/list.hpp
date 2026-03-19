#pragma once
#include <filesystem>
#include <vector>

namespace vifo::path {
namespace fs = std::filesystem;

struct List {
	fs::path scan_path{};
	// Nested entries should appear before their parents.
	std::vector<fs::path> paths{};
};
} // namespace vifo::path
