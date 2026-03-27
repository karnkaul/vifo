#include "vifo/manifest.hpp"
#include "detail/common.hpp"
#include "vifo/path/transformer.hpp"
#include "vifo/types.hpp"
#include "vifo/util/util.hpp"
#include <array>
#include <filesystem>
#include <string_view>
#include <unordered_set>

namespace vifo {
namespace {
void per_row(fs::path const& parent, std::vector<std::string>& row, fs::path const& path, MediaFileType const type, bool const mark_existing) {
	std::string_view const prefix = mark_existing && fs::exists(path) ? "*" : "";
	auto relative_path = std::format("{}{}", prefix, util::to_relative(parent, path).generic_string());
	row.push_back(std::move(relative_path));
	row.emplace_back(media_file_type_name_map.to_name(type));
}

struct PerSource {
	void operator()(std::vector<std::string>& row, Manifest::Entry const& entry) const { per_row(parent, row, entry.source, entry.type, false); }
	fs::path const& parent;
};

struct PerDestination {
	void operator()(std::vector<std::string>& row, Manifest::Entry const& entry) const { per_row(parent, row, entry.destination, entry.type, true); }
	fs::path const& parent;
};
} // namespace

auto Manifest::compute_metrics() const -> Metrics {
	auto ret = Metrics{};
	auto paths_set = std::unordered_set<fs::path>{};
	for (auto const& entry : entries) {
		if (entry.destination.empty()) { continue; }
		if (fs::exists(entry.destination)) { ++ret.existing; }
		if (paths_set.contains(entry.destination)) { ++ret.duplicates; }
		paths_set.insert(entry.destination);
	}
	return ret;
}

auto Manifest::format_destinations_table() const -> std::string {
	if (entries.empty()) { return {}; }
	static constexpr auto headers_v = std::array{"destination", "type"};
	return util::format_enumerated_table(headers_v, entries, PerDestination{.parent = parent});
}

auto Manifest::format_sources_table() const -> std::string {
	if (entries.empty()) { return {}; }
	static constexpr auto headers_v = std::array{"source", "type"};
	return util::format_enumerated_table(headers_v, entries, PerSource{.parent = parent});
}

auto Manifest::format_entries_tables() const -> std::string {
	if (entries.empty()) { return {}; }
	auto ret = format_sources_table();
	ret.append(format_destinations_table());
	return ret;
}

auto Manifest::format_orphans_table() const -> std::string {
	if (orphans.empty()) { return {}; }
	static constexpr auto headers_v = std::array{"orphan", "type"};
	return util::format_enumerated_table(headers_v, orphans, PerSource{.parent = parent});
}

auto Manifest::Transformer::transform_manifest(Manifest const& manifest, Operation const operation, bool const overwrite) const -> Transaction {
	auto ret = Transaction{.parent = manifest.parent};
	auto const transformer = path::Transformer{.overwrite = overwrite};
	for (auto const& entry : manifest.entries) {
		auto const outcome = transformer.transform(entry.source, entry.destination, operation);
		auto record = detail::to_record(entry.source, entry.destination, operation);
		on_transformed(record, outcome);
		ret.triage_record(std::move(record), outcome);
	}
	return ret;
}
} // namespace vifo
