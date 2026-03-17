#pragma once
#include <filesystem>
#include <vector>

namespace vifo {
namespace fs = std::filesystem;

struct Manifest {
	struct Entry {
		fs::path source{};
		fs::path destination{};
		bool exists{};
	};

	fs::path root{};
	std::vector<Entry> entries{};
	std::uint64_t collision_count{};
};
} // namespace vifo
