#include "vifo/generator/movie_generator.hpp"
#include "detail/common.hpp"
#include "detail/media_directory.hpp"
#include "vifo/formatter/subtitle_formatter.hpp"
#include "vifo/manifest.hpp"
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
	auto source = detail::MediaDirectory::scan_directory(directory);
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
	auto path = detail::if_directory(directory);
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

	m_movie_formatter.set_title(std::move(omdb_movie->payload.title));
	m_movie_formatter.set_year(omdb_movie->payload.year);

	auto builder = create_builder(m_movie_formatter, path->parent_path());
	builder.process_video(std::move(movie_directory.video), movie_directory.subtitles);

	return std::move(builder.manifest);
}
} // namespace vifo
