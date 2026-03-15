#pragma once
#include <filesystem>
#include <vector>

namespace vifo {
namespace fs = std::filesystem;

struct Manifest {
	struct Entry {
		fs::path source{};
		fs::path destination{};
	};

	fs::path root{};
	std::vector<Entry> entries{};
};
} // namespace vifo
