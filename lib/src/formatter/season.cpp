#include "vifo/formatter/season.hpp"
#include "detail/common.hpp"
#include "detail/media_directory.hpp"
#include "vifo/interpolator/subtitle.hpp"
#include "vifo/manifest.hpp"
#include "vifo/media_file.hpp"
#include "vifo/omdb.hpp"
#include "vifo/types.hpp"
#include <expected>
#include <filesystem>

namespace vifo::formatter {
namespace {
struct SeasonDirectory {
	struct Episode {
		MediaFile video{};
		std::vector<MediaFile> subtitles{};
	};

	std::vector<Episode> episodes{};
};

[[nodiscard]] auto build_season_directory(fs::path const& directory) -> SeasonDirectory {
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

	auto ret = SeasonDirectory{};
	ret.episodes.reserve(videos.size());

	auto episode = SeasonDirectory::Episode{};
	auto episode_stem = std::string{};
	auto const transfer_subtitles = [&](MediaFile& subtitle) {
		auto const path = subtitle.path.generic_string();
		if (!path.contains(episode_stem)) { return false; }
		episode.subtitles.push_back(std::move(subtitle));
		return true;
	};
	for (auto& video : videos) {
		episode = SeasonDirectory::Episode{.video = std::move(video)};
		episode_stem = episode.video.path.stem().string();
		std::erase_if(subtitles, transfer_subtitles);
		ret.episodes.push_back(std::move(episode));
	}

	return ret;
}
} // namespace

auto Season::create(omdb::IService const& omdb_service, Format const& format) -> Result<Season> {
	auto ret = Season{omdb_service};
	return interpolator::Season::create(format.season)
		.and_then([&](interpolator::Season season) {
			ret.m_season = std::move(season);
			return interpolator::Subtitle::create(format.subtitle);
		})
		.transform([&](interpolator::Subtitle subtitle) {
			ret.m_subtitle = std::move(subtitle);
			return std::move(ret);
		});
}

auto Season::generate_manifest(fs::path const& directory) -> Result<Manifest> {
	auto path = detail::if_directory(directory);
	if (!path) { return std::unexpected{std::move(path.error())}; }

	auto season_metadata = get_metadata(*path);
	if (!season_metadata.season_id) { return detail::to_error(Error::Type::Identify, std::format("failed to extract SeasonId: '{}'", path->generic_string())); }

	auto season_directory = build_season_directory(*path);
	if (season_directory.episodes.empty()) {
		return detail::to_error(Error::Type::Identify, std::format("no episodes found in: '{}'", path->generic_string()));
	}

	if (season_metadata.title.empty()) {
		auto const& first_episode = season_directory.episodes.front();
		auto episode_metadata = get_metadata(first_episode.video.path);
		if (episode_metadata.title.empty()) {
			return detail::to_error(Error::Type::Identify, std::format("failed to identify series title for: '{}'", path->generic_string()));
		}

		season_metadata.title = std::move(episode_metadata.title);
	}

	auto omdb_season = m_omdb_service->search_season(season_metadata.title, season_metadata.season_id->number);
	if (!omdb_season) { return detail::to_error(Error::Type::Http, omdb_season.error().text); }

	if (omdb_season->payload.title.empty()) {
		return detail::to_error(Error::Type::Identify, std::format("failed to identify series title for: '{}'", path->generic_string()));
	}

	m_season.set_season(std::move(omdb_season->payload));

	auto builder = create_builder(m_season, path->parent_path());
	for (auto& episode : season_directory.episodes) { builder.process_video(std::move(episode.video), episode.subtitles); }

	return std::move(builder.manifest);
}
} // namespace vifo::formatter
