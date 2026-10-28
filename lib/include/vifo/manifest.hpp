#pragma once
#include "vifo/transaction.hpp"
#include "vifo/types.hpp"
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace vifo {
namespace fs = std::filesystem;

struct Manifest {
	struct Metrics {
		std::int64_t existing{};
		std::int64_t duplicates{};
	};

	struct Entry {
		fs::path source{};
		fs::path destination{};
		MediaFileType type{};
	};

	using OnTransformed = std::move_only_function<void(Record const&, Outcome)>;

	[[nodiscard]] auto compute_metrics() const -> Metrics;

	[[nodiscard]] auto format_destinations_table() const -> std::string;
	[[nodiscard]] auto format_sources_table() const -> std::string;
	[[nodiscard]] auto format_entries_tables() const -> std::string;
	[[nodiscard]] auto format_orphans_table() const -> std::string;

	[[nodiscard]] auto transform(Operation operation, bool overwrite, OnTransformed on_transformed = {}) const -> Transaction;

	fs::path parent{};
	std::vector<Entry> entries{};
	std::vector<Entry> orphans{};
};
} // namespace vifo
