#pragma once
#include <span>
// #include "vifo/types.hpp"
// #include <optional>
#include "klib/cli/text_table.hpp"
#include <algorithm>
#include <array>
#include <filesystem>
#include <format>
#include <ranges>
#include <string>
#include <string_view>

namespace vifo::util {
namespace fs = std::filesystem;

[[nodiscard]] auto to_int(std::string_view text, int fallback = 0) -> int;

void join_to(std::string& out, std::string_view item, std::string_view delim = " ");
[[nodiscard]] auto join(std::span<std::string_view const> items, std::string_view delim = " ") -> std::string;

[[nodiscard]] auto path_if_exists(std::string_view path) -> fs::path;
[[nodiscard]] auto path_if_directory(std::string_view path) -> fs::path;
[[nodiscard]] auto to_relative(fs::path const& parent, fs::path const& target) -> fs::path;
auto ghost_copy(fs::path const& source, fs::path const& destination, bool overwrite) -> std::int64_t;

[[nodiscard]] auto prefix_parent(fs::path const& parent_source, fs::path const& target) -> fs::path;
[[nodiscard]] auto concat_path(fs::path const& prefix, fs::path const& subpath) -> fs::path;

[[nodiscard]] auto identify_title(fs::path const& path) -> std::string;
[[nodiscard]] auto trim_identified_title(std::string_view& out_text) -> std::string;

template <typename ContainerT, std::equality_comparable Type>
constexpr auto match_any(ContainerT const& container, Type const& t) {
	return std::ranges::find(container, t) != std::end(container);
}

constexpr auto video_extensions_v = std::array{
	".mp4", ".mkv", ".avi", ".m4v", ".webm",
};
constexpr auto is_video_file(std::string_view const extension) { return match_any(video_extensions_v, extension); }

constexpr auto subtitle_extensions_v = std::array{
	".srt",
};
constexpr auto is_subtitle_file(std::string_view const extension) { return match_any(subtitle_extensions_v, extension); }

// [[nodiscard]] auto is_year(std::string_view word) -> bool;

// [[nodiscard]] auto is_season_directory(fs::path const& path) -> bool;
// [[nodiscard]] auto is_subtitle_directory(fs::path const& path) -> bool;
// [[nodiscard]] auto is_episode(fs::path const& path) -> bool;

// [[nodiscard]] auto extract_season_id(std::string const& name) -> std::optional<SeasonId>;
// [[nodiscard]] auto extract_episode_id(std::string const& name) -> std::optional<EpisodeId>;

template <typename HeadersT, typename ContainerT, typename ProjT>
[[nodiscard]] auto format_enumerated_table(HeadersT const& headers, ContainerT const& entries, ProjT per_entry) -> std::string {
	if (entries.empty()) { return {}; }

	auto builder = klib::TextTable::Builder{};
	builder.add_column("#", klib::TextTable::Align::Right);
	for (auto const& header : headers) { builder.add_column(std::string{header}); }
	auto table = builder.build();

	auto row = std::vector<std::string>{};
	for (auto const [index, entry] : std::views::enumerate(entries)) {
		row.reserve(headers.size() + 1);
		row.push_back(std::format("{}", index + 1));
		per_entry(row, entry);
		table.push_row(std::move(row));
	}
	return table.serialize();
}
} // namespace vifo::util
