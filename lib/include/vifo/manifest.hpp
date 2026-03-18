#pragma once
#include "vifo/path/list.hpp"
#include <cstdint>
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

	[[nodiscard]] static auto build(IFormatter& formatter, path::List path_list) -> Manifest;

	[[nodiscard]] auto format_table() const -> std::string;

	fs::path parent{};
	std::vector<Entry> entries{};
	std::int64_t collision_count{};
};
} // namespace vifo
