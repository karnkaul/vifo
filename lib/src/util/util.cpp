// #include "detail/title_parser.hpp"
// #include <optional>
// #include <regex>
#include "vifo/util/util.hpp"
#include "log.hpp"
#include <filesystem>
#include <fstream>
#include <string_view>

namespace vifo {
namespace util {
namespace {
[[nodiscard]] auto mkdir(fs::path const& path) -> bool {
	if (path.empty()) { return true; }
	if (fs::exists(path)) { return fs::is_directory(path); }
	return fs::create_directories(path);
}
} // namespace
} // namespace util

auto util::get_env_var(klib::CString const key) -> klib::CString {
	if (key.as_view().empty()) { return {}; }
	// NOLINTNEXTLINE(concurrency-mt-unsafe)
	return std::getenv(key.c_str());
}

auto util::path_if_exists(std::string_view const path) -> fs::path {
	if (path.empty()) {
		log.error("empty path");
		return {};
	}

	auto ret = fs::path{path};
	if (!fs::exists(ret)) {
		log.error("nonexistent path: '{}'", path);
		return {};
	}

	return ret;
}

auto util::path_if_directory(std::string_view const path) -> fs::path {
	auto ret = path_if_exists(path);
	if (ret.empty()) { return {}; }

	if (!fs::is_directory(ret)) {
		log.error("not a directory: '{}'", path);
		return {};
	}

	return ret;
}

auto util::ghost_copy(fs::path const& source, fs::path const& destination, bool const overwrite) -> std::int64_t {
	auto const root = destination / source.filename();
	if (!mkdir(root)) { return -1; }

	auto ret = std::int64_t{};
	for (auto const& it : fs::recursive_directory_iterator{source}) {
		if (!it.is_regular_file()) { continue; }

		auto const relative = fs::relative(it.path(), source);
		auto const path = root / relative;
		if (fs::exists(path) && !overwrite) { return -1; }
		if (!mkdir(path.parent_path())) { return -1; }
		auto file = std::ofstream{path, std::ios::out};
		if (!file) { return -1; }
		++ret;
	}
	return ret;
}

auto util::prefix_parent(fs::path const& parent_source, fs::path const& target) -> fs::path {
	if (!parent_source.has_parent_path()) { return target; }
	return parent_source.parent_path() / target;
}

auto util::to_int(std::string_view const text, int const fallback) -> int {
	if (text.empty()) { return fallback; }
	auto ret = int{};
	// NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
	auto const* end = text.data() + text.size();
	auto const [_, ec] = std::from_chars(text.data(), end, ret);
	if (ec != std::errc{}) { return fallback; }
	return ret;
}

void util::join_to(std::string& out, std::string_view const item, std::string_view const delim) {
	if (!out.empty()) { out += delim; }
	out += item;
}

auto util::join(std::span<std::string_view const> items, std::string_view const delim) -> std::string {
	auto ret = std::string{};
	for (auto const item : items) { join_to(ret, item, delim); }
	return ret;
}

// auto util::is_year(std::string_view const word) -> bool {
// 	static auto const s_regex = std::regex{R"([1-3][0-9]{3})"};
// 	// NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
// 	return std::regex_match(word.data(), word.data() + word.size(), s_regex);
// }

// auto util::is_season_directory(fs::path const& path) -> bool {
// 	if (!fs::is_directory(path)) { return {}; }
// 	static auto const s_regex_1 = std::regex{R"(.*Season.[0-9]{2}.*)"};
// 	static auto const s_regex_2 = std::regex{R"(.*S[0-9]{2}.*)"};
// 	auto const filename = fs::canonical(path).stem().string();
// 	return std::regex_match(filename, s_regex_1) || std::regex_match(filename, s_regex_2);
// }

// auto util::is_subtitle_directory(fs::path const& path) -> bool {
// 	if (!fs::is_directory(path)) { return false; }
// 	auto const filename = path.filename().string();
// 	static constexpr auto subtitle_directories_v = std::array{
// 		"subs",
// 		"subtitles",
// 		"Subs",
// 		"Subtitles",
// 	};
// 	return std::ranges::find(subtitle_directories_v, filename) != subtitle_directories_v.end();
// }

// auto util::is_episode(fs::path const& path) -> bool {
// 	if (!fs::exists(path)) { return {}; }
// 	static auto const s_regex = std::regex{R"(.*S[0-9]{2}E[0-9]{2}.*)"};
// 	return std::regex_match(fs::canonical(path).string(), s_regex);
// }

// auto util::identify_title(fs::path const& path) -> std::string {
// 	if (!fs::exists(path)) { return {}; }
// 	auto const stem = fs::canonical(path).stem();
// 	return detail::TitleParser{}.parse(stem.string());
// }

// auto util::extract_season_id(std::string const& name) -> std::optional<SeasonId> {
// 	if (name.empty()) { return {}; }

// 	static auto const s_regex_1 = std::regex{R"(Season.[0-9]{2})"};
// 	static auto const s_regex_2 = std::regex{R"(S[0-9]{2})"};
// 	auto matches = std::smatch{};
// 	if (!std::regex_search(name, matches, s_regex_1) && !std::regex_search(name, matches, s_regex_2)) { return {}; }
// 	auto const str = std::string{matches[0]};
// 	auto const number = to_int(std::string_view{str}.substr(str.size() - 2));
// 	if (number <= 0) { return {}; }
// 	return SeasonId{number};
// }

// auto util::extract_episode_id(std::string const& name) -> std::optional<EpisodeId> {
// 	if (name.empty()) { return {}; }

// 	static auto const s_regex = std::regex{R"(S[0-9]{2}E[0-9]{2})"};
// 	auto matches = std::smatch{};
// 	if (!std::regex_search(name, matches, s_regex)) { return {}; }
// 	auto const str = std::string{matches[0]};
// 	auto const number = to_int(std::string_view{str}.substr(str.size() - 2));
// 	auto const season = to_int(std::string_view{str}.substr(1, 2));
// 	if (number <= 0 || season <= 0) { return {}; }
// 	return EpisodeId{season, number};
// }
} // namespace vifo
