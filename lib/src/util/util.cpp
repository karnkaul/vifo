#include "vifo/util/util.hpp"
#include "detail/title_parser.hpp"
#include "log.hpp"
#include "vifo/types.hpp"
#include <array>
#include <filesystem>
#include <fstream>
#include <optional>
#include <regex>
#include <string_view>

namespace vifo {
namespace util {
namespace {
[[nodiscard]] auto mkdir(fs::path const& path) -> bool {
	if (path.empty()) { return true; }
	if (fs::exists(path)) { return fs::is_directory(path); }
	return fs::create_directories(path);
}

template <std::size_t N>
struct Matcher {
	template <std::convertible_to<char const*>... Ts>
	explicit Matcher(Ts const... exprs) : regexes{std::regex{exprs}...} {}

	[[nodiscard]] auto is_match(std::string_view const text) const -> bool {
		auto const pred = [text](std::regex const& r) { return std::regex_match(text.data(), text.data() + text.size(), r); };
		return std::ranges::any_of(regexes, pred);
	}

	[[nodiscard]] auto search(std::smatch& out, std::string const& text) const -> bool {
		auto const pred = [&](std::regex const& r) { return std::regex_search(text, out, r); };
		return std::ranges::any_of(regexes, pred);
	}

	std::array<std::regex, N> regexes{};
};

template <typename... Ts>
Matcher(Ts...) -> Matcher<sizeof...(Ts)>;

namespace matcher {
auto const year = Matcher{R"([1-9]\d{3}(?!\d).*)"};
auto const resolution = Matcher{R"(\d{3}\d?p)"};
auto const episode_id = Matcher{R"(S\d{2}E\d{2})"};
auto const season_id = Matcher{R"(Season.\d{2})", R"(S\d{2})"};
} // namespace matcher
} // namespace
} // namespace util

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

auto util::to_relative(fs::path const& parent, fs::path const& target) -> fs::path {
	if (parent.empty()) { return target; }
	return fs::relative(target, parent);
}

void util::sanitize_for_path(std::string& out, char const replace) {
	static constexpr auto forbidden_v = std::array{'<', '>', ':', '\"', '|', '?', '*'};
	for (char& c : out) {
		if (match_any(forbidden_v, c)) { c = replace; }
	}
}

auto util::ghost_copy(fs::path const& source, fs::path const& destination, bool const overwrite) -> std::int64_t {
	auto const root = destination / source.filename();
	if (!mkdir(root)) { return -1; }

	auto ret = std::int64_t{};
	for (auto const& it : fs::recursive_directory_iterator{source}) {
		auto const relative = fs::relative(it.path(), source);
		auto const path = root / relative;

		if (fs::exists(path) && !overwrite) { return -1; }

		if (it.is_directory()) {
			if (!mkdir(it.path())) { return -1; }
			++ret;
			continue;
		}

		if (!it.is_regular_file()) { continue; }

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

auto util::concat_path(fs::path const& prefix, fs::path const& subpath) -> fs::path {
	if (prefix.empty()) { return subpath; }
	return prefix / subpath;
}

auto util::identify_title(fs::path const& path) -> std::string {
	if (!fs::exists(path)) { return {}; }
	auto const stem = fs::canonical(path).stem();
	return detail::TitleParser{}.parse(stem.string());
}

auto util::trim_identified_title(std::string_view& out_text) -> std::string { return detail::TitleParser{}.parse_and_trim(out_text); }

auto util::trim_identified_year(std::string_view& out_text) -> std::string {
	if (out_text.size() < 4) { return {}; }
	if (!matcher::year.is_match(out_text)) { return {}; }
	auto ret = std::string{out_text.substr(0, 4)};
	out_text.remove_prefix(ret.size());
	return ret;
}

auto util::ignore_for_title(std::string_view word) -> bool {
	if (!trim_identified_year(word).empty()) { return true; }
	return matcher::resolution.is_match(word) || matcher::episode_id.is_match(word);
}

auto util::get_media_file_type(fs::path const& file) -> std::optional<MediaFileType> {
	struct MFTExtension {
		MediaFileType type{};
		std::vector<std::string_view> extensions{};
	};
	static auto const s_mft_extensions = std::array{
		MFTExtension{.type = MediaFileType::Video, .extensions = {".mp4", ".mkv", ".avi", ".m4v", ".webm"}},
		MFTExtension{.type = MediaFileType::Subtitle, .extensions = {".srt"}},
	};

	auto const extension = file.extension().string();
	for (auto const& mfte : s_mft_extensions) {
		if (match_any(mfte.extensions, extension)) { return mfte.type; }
	}
	return {};
}

auto util::extract_season_id(std::string const& name) -> std::optional<SeasonId> {
	if (name.empty()) { return {}; }

	auto matches = std::smatch{};
	if (!matcher::season_id.search(matches, name)) { return {}; }
	auto const str = std::string{matches[0]};
	auto const number = to_int(std::string_view{str}.substr(str.size() - 2));
	if (number <= 0) { return {}; }
	return SeasonId{.number = number};
}

auto util::extract_episode_id(std::string const& name) -> std::optional<EpisodeId> {
	if (name.empty()) { return {}; }

	auto matches = std::smatch{};
	if (!matcher::episode_id.search(matches, name)) { return {}; }
	auto const str = std::string{matches[0]};
	auto const number = to_int(std::string_view{str}.substr(str.size() - 2));
	auto const season = to_int(std::string_view{str}.substr(1, 2));
	if (number <= 0 || season <= 0) { return {}; }
	return EpisodeId{.season = season, .number = number};
}
} // namespace vifo
