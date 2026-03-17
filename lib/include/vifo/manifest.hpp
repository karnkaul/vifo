#pragma once
#include <cstdint>
#include <filesystem>
#include <vector>

namespace vifo {
class IFormatter;

namespace fs = std::filesystem;

struct Manifest {
	struct Entry {
		fs::path source{};
		fs::path destination{};
	};

	[[nodiscard]] static auto build(IFormatter& formatter, fs::path const& root) -> Manifest;
	static void append_to(Manifest& out, IFormatter& formatter, fs::path const& subdirectory);

	fs::path root{};
	std::vector<Entry> entries{};
	std::int64_t collision_count{};
};
} // namespace vifo
