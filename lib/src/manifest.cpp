#include "vifo/manifest.hpp"
#include "klib/text_table.hpp"
#include "vifo/formatter.hpp"
#include "vifo/util/util.hpp"
#include <filesystem>
#include <ranges>
#include <string_view>

namespace vifo {
namespace {
void adjust_parent(fs::path& out_parent, fs::path const& entry) {
	if (out_parent.empty()) {
		out_parent = entry.parent_path();
		return;
	}

	if (out_parent != entry) { return; }
	out_parent = entry.parent_path();
}
} // namespace

auto Manifest::build(IFormatter& formatter, std::span<fs::path> sources) -> Manifest {
	auto ret = Manifest{};
	for (auto& source : sources) {
		auto const src_filename = source.filename().string();
		auto const dst_filename = formatter.format(src_filename);
		if (dst_filename.empty()) { continue; }

		auto destination = util::prefix_parent(source, dst_filename);
		if (fs::exists(destination)) { ++ret.collision_count; }
		adjust_parent(ret.parent, source);
		ret.entries.push_back(Manifest::Entry{.source = std::move(source), .destination = std::move(destination)});
	}
	return ret;
}

auto Manifest::serialize_to_table() const -> std::string {
	if (entries.empty()) { return {}; }

	auto table = klib::TextTable::Builder{}.add_column("#", klib::TextTable::Align::Right).add_column("destination").add_column("source").build();
	auto const to_relative = [&](fs::path const& path) {
		if (parent.empty()) { return path; }
		return fs::relative(path, parent);
	};

	auto row = std::vector<std::string>{};
	for (auto const [index, entry] : std::views::enumerate(entries)) {
		row.reserve(3);
		std::string_view const prefix = fs::exists(entry.destination) ? "*" : "";
		row.push_back(std::format("{}{}", prefix, index + 1));
		row.push_back(to_relative(entry.destination).generic_string());
		row.push_back(to_relative(entry.source).generic_string());
		table.push_row(std::move(row));
	}

	return table.serialize();
}
} // namespace vifo
