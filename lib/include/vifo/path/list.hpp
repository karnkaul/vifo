#pragma once
#include <filesystem>
#include <string>
#include <vector>

namespace vifo::path {
namespace fs = std::filesystem;

struct List {
	[[nodiscard]] auto format_table() const -> std::string;

	fs::path scan_path{};
	// Nested entries should appear before their parents.
	std::vector<fs::path> paths{};
};
} // namespace vifo::path
