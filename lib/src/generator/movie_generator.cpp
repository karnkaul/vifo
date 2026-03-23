#include "vifo/generator/movie_generator.hpp"
#include "detail/common.hpp"
#include "vifo/formatter/subtitle_formatter.hpp"
#include "vifo/manifest.hpp"
#include "vifo/media/directory.hpp"
#include "vifo/types.hpp"
#include "vifo/util/util.hpp"
#include <algorithm>
#include <filesystem>

namespace vifo {
namespace {
struct MovieDirectory {
	MediaFile video{};
	std::vector<MediaFile> subtitles{};
};

[[nodiscard]] auto build_movie_directory(fs::path const& directory) -> MovieDirectory {
	auto source = MediaDirectory::scan_directory(directory);
	auto videos = std::vector<MediaFile>{};
	auto subtitles = std::vector<MediaFile>{};
	for (auto& file : source.files) {
		switch (file.type) {
		case MediaFileType::Video: videos.push_back(std::move(file)); break;
		case MediaFileType::Subtitle: subtitles.push_back(std::move(file)); break;
		default: break;
		}
	}

	if (videos.empty()) { return {}; }

	std::ranges::sort(videos, [](MediaFile const& a, MediaFile const& b) { return a.size > b.size; });
	return MovieDirectory{.video = std::move(videos.front()), .subtitles = std::move(subtitles)};
}
} // namespace

auto MovieGenerator::create(omdb::IService const& omdb_service, Format const& format) -> Result<MovieGenerator> {
	auto ret = MovieGenerator{omdb_service};
	return TitleFormatter::create(format.movie)
		.and_then([&](TitleFormatter formatter) {
			ret.m_movie_formatter = std::move(formatter);
			return SubtitleFormatter::create(format.subtitle);
		})
		.transform([&](SubtitleFormatter formatter) {
			ret.m_subtitle_formatter = std::move(formatter);
			return std::move(ret);
		});
}

auto MovieGenerator::generate_manifest(fs::path const& directory) -> Result<Manifest> {
	auto path = if_directory(directory);
	if (!path) { return std::unexpected{std::move(path.error())}; }

	auto movie_directory = build_movie_directory(*path);
	if (movie_directory.video.path.empty()) {
		return detail::to_error(Error::Type::Identify, std::format("no video files found in: '{}'", path->generic_string()));
	}

	auto const title = util::identify_title(movie_directory.video.path);
	if (title.empty()) {
		return detail::to_error(Error::Type::Identify, std::format("failed to identify title for: '{}'", movie_directory.video.path.generic_string()));
	}

	auto omdb_movie = m_omdb_service->search_movie(title);
	if (!omdb_movie) { return detail::to_error(Error::Type::Http, omdb_movie.error().text); }

	if (omdb_movie->payload.title.empty()) {
		return detail::to_error(Error::Type::Identify, std::format("failed to identify title for: '{}'", movie_directory.video.path.generic_string()));
	}

	m_subtitle_formatter.set_title(omdb_movie->payload.title);
	m_subtitle_formatter.set_number(0);
	m_movie_formatter.set_title(std::move(omdb_movie->payload.title));
	m_movie_formatter.set_year(omdb_movie->payload.year);

	auto ret = Manifest{};
	detail::filter_en_subtitles(ret, movie_directory.subtitles);

	auto video_entry = Manifest::Entry{.source = std::move(movie_directory.video.path), .type = movie_directory.video.type};
	video_entry.destination = m_movie_formatter.format_video(video_entry.source);
	if (video_entry.destination.empty()) {
		ret.orphans.push_back(std::move(video_entry));
		for (auto& subtitle : movie_directory.subtitles) { ret.orphans.push_back(Manifest::Entry{.source = std::move(subtitle.path), .type = subtitle.type}); }
	} else {
		auto const subtitle_directory = get_subtitles_dir_for(video_entry.destination);
		ret.entries.push_back(std::move(video_entry));

		for (auto& subtitle : movie_directory.subtitles) {
			auto subtitle_entry = Manifest::Entry{.source = std::move(subtitle.path), .type = subtitle.type};
			subtitle_entry.destination = m_subtitle_formatter.format_path(subtitle_directory, subtitle_entry.source.extension().string());
			ret.entries.push_back(std::move(subtitle_entry));
		}
	}

	return ret;
}
} // namespace vifo
