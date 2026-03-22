#include "vifo/manifest.hpp"
#include "detail/common.hpp"
#include "vifo/path/transformer.hpp"
#include "vifo/util/util.hpp"
#include <array>
#include <filesystem>
#include <unordered_set>

namespace vifo {
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
