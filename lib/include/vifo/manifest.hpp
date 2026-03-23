#pragma once
#include "klib/base_types.hpp"
#include "vifo/transaction.hpp"
#include "vifo/types.hpp"
#include <cstdint>
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

	class Transformer;

	[[nodiscard]] auto compute_metrics() const -> Metrics;

	[[nodiscard]] auto format_destinations_table() const -> std::string;
	[[nodiscard]] auto format_sources_table() const -> std::string;
	[[nodiscard]] auto format_entries_tables() const -> std::string;
	[[nodiscard]] auto format_orphans_table() const -> std::string;

	fs::path parent{};
	std::vector<Entry> entries{};
	std::vector<Entry> orphans{};
};

class Manifest::Transformer : public klib::Polymorphic {
  public:
	[[nodiscard]] auto transform_manifest(Manifest const& manifest, Operation operation, bool overwrite) const -> Transaction;

  protected:
	virtual void on_transformed([[maybe_unused]] Record const& record, [[maybe_unused]] Outcome outcome) const {}
};
} // namespace vifo
