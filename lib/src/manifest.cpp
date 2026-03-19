#include "vifo/manifest.hpp"
#include "detail/common.hpp"
#include "klib/cli/text_table.hpp"
#include "vifo/formatter.hpp"
#include "vifo/path/list.hpp"
#include "vifo/path/transformer.hpp"
#include "vifo/util/util.hpp"
#include <filesystem>
#include <ranges>
#include <string_view>
#include <unordered_set>

namespace vifo {
auto Manifest::build(IFormatter& formatter, path::List path_list) -> Manifest {
	auto destinations = std::unordered_set<fs::path>{};
	auto ret = Manifest{.parent = std::move(path_list.scan_path)};
	for (auto& source : path_list.paths) {
		if (source.empty()) { continue; }

		auto const src_stem = source.stem().string();
		auto dst_filename = formatter.format(src_stem);
		if (dst_filename.empty()) { continue; }
		dst_filename += source.extension();

		auto destination = util::prefix_parent(source, dst_filename);
		if (fs::exists(destination)) { ++ret.metrics.existing; }
		destinations.insert(destination);

		if (source == ret.parent) { ret.parent = ret.parent.parent_path(); }
		ret.entries.push_back(Manifest::Entry{.source = std::move(source), .destination = std::move(destination)});
	}
	ret.metrics.duplicates = std::int64_t(ret.entries.size() - destinations.size());
	return ret;
}

auto Manifest::transform(Operation const operation, bool const overwrite) const -> Transaction {
	auto ret = Transaction{.parent = parent};
	auto const transformer = path::Transformer{.overwrite = overwrite};
	for (auto const& entry : entries) {
		auto const outcome = transformer.transform(entry.source, entry.destination, operation);
		auto record = detail::to_record(entry.source, entry.destination, operation);
		ret.triage_record(std::move(record), outcome);
	}
	return ret;
}

auto Manifest::format_table() const -> std::string {
	if (entries.empty()) { return {}; }

	auto table = klib::TextTable::Builder{}.add_column("#", klib::TextTable::Align::Right).add_column("destination").add_column("source").build();
	auto row = std::vector<std::string>{};
	for (auto const [index, entry] : std::views::enumerate(entries)) {
		row.reserve(3);
		std::string_view const prefix = fs::exists(entry.destination) ? "*" : "";
		row.push_back(std::format("{}{}", prefix, index + 1));
		row.push_back(util::to_relative(parent, entry.destination).generic_string());
		row.push_back(util::to_relative(parent, entry.source).generic_string());
		table.push_row(std::move(row));
	}

	return table.serialize();
}
} // namespace vifo
