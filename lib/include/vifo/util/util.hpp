#pragma once
#include "klib/c_string.hpp"
#include <span>
// #include "vifo/types.hpp"
// #include <optional>
#include "vifo/manifest.hpp"
#include "vifo/record.hpp"
#include <algorithm>
#include <array>
#include <filesystem>
#include <format>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

namespace vifo::util {
namespace fs = std::filesystem;

[[nodiscard]] auto get_env_var(klib::CString key) -> klib::CString;

[[nodiscard]] auto to_int(std::string_view text, int fallback = 0) -> int;

void join_to(std::string& out, std::string_view item, std::string_view delim = " ");
[[nodiscard]] auto join(std::span<std::string_view const> items, std::string_view delim = " ") -> std::string;

[[nodiscard]] auto path_if_exists(std::string_view path) -> fs::path;
[[nodiscard]] auto path_if_directory(std::string_view path) -> fs::path;
auto ghost_copy(fs::path const& source, fs::path const& destination, bool overwrite) -> std::int64_t;

[[nodiscard]] auto prefix_parent(fs::path const& parent_source, fs::path const& target) -> fs::path;
[[nodiscard]] auto concat_path(fs::path const& prefix, fs::path const& subpath) -> fs::path;

constexpr auto video_extensions_v = std::array{
	".mp4", ".mkv", ".avi", ".m4v", ".webm",
};
constexpr auto is_video_file(std::string_view const extension) { return std::ranges::find(video_extensions_v, extension) != video_extensions_v.end(); }

constexpr auto subtitle_extensions_v = std::array{
	".srt",
};
constexpr auto is_subtitle_file(std::string_view const extension) { return std::ranges::find(subtitle_extensions_v, extension) != subtitle_extensions_v.end(); }

[[nodiscard]] auto transform(Manifest const& manifest, Operation operation, bool overwrite = false) -> std::vector<Record>;
[[nodiscard]] auto undo(std::span<Record const> records) -> std::vector<Record>;

// [[nodiscard]] auto is_year(std::string_view word) -> bool;

// [[nodiscard]] auto is_season_directory(fs::path const& path) -> bool;
// [[nodiscard]] auto is_subtitle_directory(fs::path const& path) -> bool;
// [[nodiscard]] auto is_episode(fs::path const& path) -> bool;

// [[nodiscard]] auto identify_title(fs::path const& path) -> std::string;
// [[nodiscard]] auto extract_season_id(std::string const& name) -> std::optional<SeasonId>;
// [[nodiscard]] auto extract_episode_id(std::string const& name) -> std::optional<EpisodeId>;

template <typename ContainerT, typename ProjT>
void serialize_enumerated_to(std::string& out, ContainerT const& container, ProjT proj) {
	if (container.empty()) { return; }

	auto const width = [&] {
		if (container.size() < 10) { return 1; }
		if (container.size() < 100) { return 2; }
		return 3;
	}();

	for (auto const [index, t] : std::views::enumerate(container)) { std::format_to(std::back_inserter(out), "{:>{}}. {}\n", index + 1, width, proj(t)); }
	out.pop_back();
}
} // namespace vifo::util
