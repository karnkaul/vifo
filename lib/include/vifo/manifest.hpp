#pragma once
#include <cstdint>
#include <filesystem>
#include <span>
#include <vector>

namespace vifo {
class IFormatter;

namespace fs = std::filesystem;

struct Manifest {
	struct Entry {
		fs::path source{};
		fs::path destination{};
	};

	[[nodiscard]] static auto build(IFormatter& formatter, std::span<fs::path> sources) -> Manifest;

	[[nodiscard]] auto serialize_to_table() const -> std::string;

	fs::path parent{};
	std::vector<Entry> entries{};
	std::int64_t collision_count{};
};
} // namespace vifo
