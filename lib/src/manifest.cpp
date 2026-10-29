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
void per_row(std::vector<std::string>& row, fs::path const& path, MediaFileType const type, bool const mark_existing) {
	std::string_view const prefix = mark_existing && fs::exists(path) ? "*" : "";
	auto relative_path = std::format("{}{}", prefix, path.generic_string());
	row.push_back(std::move(relative_path));
	row.emplace_back(media_file_type_name_map.to_name(type));
}

struct PerSource {
	void operator()(std::vector<std::string>& row, Manifest::Entry const& entry) const { per_row(row, entry.source, entry.type, false); }
};

struct PerDestination {
	void operator()(std::vector<std::string>& row, Manifest::Entry const& entry) const { per_row(row, entry.destination, entry.type, true); }
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
	return util::format_enumerated_table(headers_v, entries, PerDestination{});
}

auto Manifest::format_sources_table() const -> std::string {
	if (entries.empty()) { return {}; }
	static constexpr auto headers_v = std::array{"source", "type"};
	return util::format_enumerated_table(headers_v, entries, PerSource{});
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
	return util::format_enumerated_table(headers_v, orphans, PerSource{});
}

auto Manifest::transform(Operation const operation, bool const overwrite, OnTransformed on_transformed) const -> Transaction {
	auto ret = Transaction{.parent = parent};
	auto const transformer = path::Transformer{.overwrite = overwrite};
	for (auto const& entry : entries) {
		auto const source = parent / entry.source;
		auto const destination = parent / entry.destination;
		auto const outcome = transformer.transform(source, destination, operation);
		auto record = detail::to_record(source, destination, operation);
		if (on_transformed) { on_transformed(record, outcome); }
		ret.triage_record(std::move(record), outcome);
	}
	return ret;
}

auto Manifest::merge_child(Manifest const& child, fs::path const& destination_directory) -> bool {
	if (!util::is_parent(parent, child.parent) || !destination_directory.is_relative()) { return false; }

	auto const source_prefix = fs::relative(child.parent, parent);
	auto const destination_prefix = util::prefix_parent(source_prefix, destination_directory);
	for (auto const& in : child.entries) {
		auto out = Entry{
			.source = source_prefix / in.source,
			.destination = destination_prefix / in.destination,
		};
		entries.push_back(std::move(out));
	}
	for (auto const& entry : child.orphans) { orphans.push_back(Entry{.source = source_prefix / entry.source}); }

	return true;
}
} // namespace vifo
