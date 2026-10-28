#include "vifo/util/util.hpp"
#include "detail/common.hpp"
#include "log.hpp"
#include "vifo/types.hpp"
#include <array>
#include <filesystem>
#include <fstream>

namespace vifo {
namespace util {
namespace {
[[nodiscard]] auto mkdir(fs::path const& path) -> bool {
	if (path.empty()) { return true; }
	if (fs::exists(path)) { return fs::is_directory(path); }
	return fs::create_directories(path);
}

auto verify_non_empty_impl(std::string_view const format, std::string_view const type_name) -> Result<void> {
	if (!format.empty()) { return {}; }
	return detail::to_error(Error::Type::Format, std::format("{} is empty", type_name));
}
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

auto util::verify_non_empty(interpolator::PatternSwapFormat const& format) -> Result<void> {
	return verify_non_empty_impl(format.input, "PatternSwawpFormat.input").and_then([&] {
		return verify_non_empty_impl(format.output, "PatternSwawpFormat.output");
	});
}

void util::ensure_nonempty(interpolator::TitleFormat& out) {
	static constexpr auto default_v = interpolator::TitleFormat{};
	if (out.directory.empty()) { out.directory = default_v.directory; }
	if (out.video.empty()) { out.video = default_v.video; }
}

void util::ensure_nonempty(interpolator::SubtitleFormat& out) {
	static constexpr auto default_v = interpolator::SubtitleFormat{};
	if (out.output.empty()) { out.output = default_v.output; }
}

void util::ensure_nonempty(interpolator::SeasonFormat& out) {
	static constexpr auto default_v = interpolator::SeasonFormat{};
	if (out.directory.empty()) { out.directory = default_v.directory; }
	if (out.titled_video.empty()) { out.titled_video = default_v.titled_video; }
	if (out.untitled_video.empty()) { out.untitled_video = default_v.untitled_video; }
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

auto util::sanitize_for_path(std::string_view const text, char const replace) -> std::string {
	static constexpr auto forbidden_v = std::array{'<', '>', ':', '\"', '|', '?', '*'};
	auto ret = std::string{};
	ret.reserve(text.size());
	for (char c : text) {
		if (match_any(forbidden_v, c)) {
			if (replace == '\0') { continue; }
			c = replace;
		}
		ret.push_back(c);
	}
	return ret;
}

auto util::ghost_copy(fs::path const& source, fs::path const& destination, bool const overwrite) -> std::int64_t {
	auto const root = destination / source.filename();
	if (!mkdir(root)) { return -1; }

	auto ret = std::int64_t{};
	for (auto const& it : fs::recursive_directory_iterator{source}) {
		auto const relative = to_relative(source, it.path());
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
} // namespace vifo
