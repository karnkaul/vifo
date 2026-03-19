#pragma once
#include "vifo/path/list.hpp"
#include <cstdint>
#include <vector>

namespace vifo {
class IFormatter;

namespace fs = std::filesystem;

struct Manifest {
	struct Metrics {
		std::int64_t existing{};
		std::int64_t duplicates{};
	};

	struct Entry {
		fs::path source{};
		fs::path destination{};
	};

	[[nodiscard]] static auto build(IFormatter& formatter, path::List path_list) -> Manifest;

	[[nodiscard]] auto format_table() const -> std::string;

	fs::path parent{};
	std::vector<Entry> entries{};
	Metrics metrics{};
};
} // namespace vifo
