#include "vifo/manifest.hpp"
#include "detail/common.hpp"
#include "vifo/formatter/formatter.hpp"
#include "vifo/path/list.hpp"
#include "vifo/path/transformer.hpp"
#include "vifo/util/util.hpp"
#include <array>
#include <filesystem>
#include <unordered_set>

namespace vifo {
auto Manifest::build(Formatter& formatter, path::List path_list) -> Manifest {
	auto destinations = std::unordered_set<fs::path>{};
	auto ret = Manifest{.parent = std::move(path_list.scan_path)};
	for (auto& source : path_list.paths) {
		if (source.empty()) { continue; }

		auto destination = formatter.format_path(source);
		if (destination.empty()) { continue; }

		if (fs::exists(destination)) { ++ret.metrics.existing; }
		destinations.insert(destination);

		if (source == ret.parent) { ret.parent = ret.parent.parent_path(); }
		ret.entries.push_back(Manifest::Entry{.source = std::move(source), .destination = std::move(destination)});
	}
	ret.metrics.duplicates = std::int64_t(ret.entries.size() - destinations.size());
	return ret;
}

auto Manifest::format_table() const -> std::string {
	static constexpr auto headers_v = std::array{
		"destination",
		"source",
	};
	auto const per_entry = [this](std::vector<std::string>& row, Entry const& entry) {
		row.push_back(util::to_relative(parent, entry.destination).generic_string());
		row.push_back(util::to_relative(parent, entry.source).generic_string());
	};
	return util::format_enumerated_table(headers_v, entries, per_entry);
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
