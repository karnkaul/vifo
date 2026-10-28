#include "detail/common.hpp"
#include "vifo/media_file.hpp"
#include "vifo/util/util.hpp"
#include <algorithm>
#include <format>
#include <ranges>

namespace vifo {
namespace {
[[nodiscard]] auto format_message(Error::Type const type, std::string_view const msg) {
	return std::format("[{}] {}", Error::type_name_map.to_name(type), msg);
}

[[nodiscard]] auto is_en_subtitle(fs::path const& path) {
	auto stem = detail::cleanup_filestem(path);
	for (char& c : stem) { c = static_cast<char>(std::tolower(static_cast<unsigned char>(c))); }

	static constexpr auto const pred = [](auto const it) {
		auto const word = std::string_view{it};
		return word == "en" || word == "eng" || word == "english";
	};

	return std::ranges::any_of(std::views::split(stem, ' '), pred);
}
} // namespace

auto detail::to_error(Error::Type type, expression::Token token, std::string_view input, std::string_view msg) -> std::unexpected<Error> {
	auto ret = Error{.type = type, .message = format_message(type, msg)};
	if (!input.empty()) { expression::Token::Highlight{}.format_to(ret.message, token, input); }
	return std::unexpected{std::move(ret)};
}

auto detail::to_error(Error::Type type, std::string_view msg) -> std::unexpected<Error> {
	return std::unexpected{Error{.type = type, .message = format_message(type, msg)}};
}

auto detail::if_directory(fs::path const& path) -> Result<fs::path> {
	if (!fs::is_directory(path)) { return to_error(Error::Type::Argument, std::format("not a directory: '{}'", path.generic_string())); }
	return fs::canonical(path);
}

auto detail::cleanup_filestem(fs::path const& path) -> std::string {
	auto ret = path.stem().generic_string();
	std::ranges::replace(ret, '.', ' ');
	return ret;
}

void detail::filter_en_subtitles(Manifest& out_manifest, std::vector<MediaFile>& out_files, fs::path const& video) {
	auto const video_stem = video.stem();
	auto const transfer = [&](MediaFile& file) {
		if (file.path.stem() == video_stem) { return false; }
		if (!is_en_subtitle(file.path)) {
			auto entry = Manifest::Entry{.source = util::to_relative(out_manifest.parent, file.path), .type = file.type};
			out_manifest.orphans.push_back(std::move(entry));
			return true;
		}
		return false;
	};
	std::erase_if(out_files, transfer);
}

void detail::ensure_relative_paths(Manifest& out_manifest) {
	if (out_manifest.parent.empty()) { return; }

	auto const ensure_relative = [&](fs::path& out_path) {
		if (out_path.is_relative()) { return; }
		out_path = util::to_relative(out_manifest.parent, out_path);
	};

	for (auto& entry : out_manifest.entries) {
		ensure_relative(entry.source);
		ensure_relative(entry.destination);
	}
	for (auto& entry : out_manifest.orphans) { ensure_relative(entry.source); }
}
} // namespace vifo
