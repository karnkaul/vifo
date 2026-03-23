#include "vifo/generator/season_generator.hpp"
#include "detail/common.hpp"
#include "vifo/formatter/subtitle_formatter.hpp"
#include "vifo/manifest.hpp"
#include "vifo/media/directory.hpp"
#include "vifo/omdb.hpp"
#include "vifo/types.hpp"
#include "vifo/util/util.hpp"
#include <expected>
#include <filesystem>

namespace vifo {
namespace {
struct SeasonDirectory {
	struct Episode {
		MediaFile video{};
		std::vector<MediaFile> subtitles{};
	};

	std::vector<Episode> episodes{};
};

[[nodiscard]] auto build_season_directory(fs::path const& directory) -> SeasonDirectory {
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

auto SeasonGenerator::create(omdb::IService const& omdb_service, Format const& format) -> Result<SeasonGenerator> {
	auto ret = SeasonGenerator{omdb_service};
	return SeasonFormatter::create(format.season)
		.and_then([&](SeasonFormatter formatter) {
			ret.m_season_formatter = std::move(formatter);
			return SubtitleFormatter::create(format.subtitle);
		})
		.transform([&](SubtitleFormatter formatter) {
			ret.m_subtitle_formatter = std::move(formatter);
			return std::move(ret);
		});
}

auto SeasonGenerator::generate_manifest(fs::path const& directory) -> Result<Manifest> {
	auto path = if_directory(directory);
	if (!path) { return std::unexpected{std::move(path.error())}; }

	auto season_id = util::extract_season_id(path->filename().string());
	if (!season_id) { return detail::to_error(Error::Type::Identify, std::format("failed to extract SeasonId: '{}'", path->generic_string())); }

	auto season_directory = build_season_directory(*path);
	if (season_directory.episodes.empty()) {
		return detail::to_error(Error::Type::Identify, std::format("no episodes found in: '{}'", path->generic_string()));
	}

	auto const& first_episode = season_directory.episodes.front();
	auto series_title = util::identify_title(first_episode.video.path);
	if (series_title.empty()) {
		return detail::to_error(Error::Type::Identify, std::format("failed to identify series title for: '{}'", path->generic_string()));
	}

	auto omdb_season = m_omdb_service->search_season(series_title, season_id->number);
	if (!omdb_season) { return detail::to_error(Error::Type::Http, omdb_season.error().text); }

	if (omdb_season->payload.title.empty()) {
		return detail::to_error(Error::Type::Identify, std::format("failed to identify title for: '{}'", first_episode.video.path.generic_string()));
	}

	series_title = omdb_season->payload.title;
	m_season_formatter.set_season(std::move(omdb_season->payload));

	auto ret = Manifest{.parent = path->parent_path()};

	auto subtitle_directory = fs::path{};
	for (auto& episode : season_directory.episodes) {
		auto destination = m_season_formatter.format_path(episode.video.path);
		auto video_entry = Manifest::Entry{.source = std::move(episode.video.path), .type = episode.video.type};
		if (destination.empty()) {
			ret.orphans.push_back(std::move(video_entry));
			for (auto& episode : season_directory.episodes) {
				for (auto& subtitle : episode.subtitles) { ret.orphans.push_back(Manifest::Entry{.source = std::move(subtitle.path), .type = subtitle.type}); }
			}
			continue;
		}

		detail::filter_en_subtitles(ret, episode.subtitles);

		video_entry.destination = m_season_formatter.format_path(video_entry.source);
		if (subtitle_directory.empty()) { subtitle_directory = util::prefix_parent(video_entry.destination, get_subtitles_dir_for(video_entry.destination)); }
		m_subtitle_formatter.set_title(video_entry.destination.stem().string());
		ret.entries.push_back(std::move(video_entry));

		for (auto& subtitle : episode.subtitles) {
			auto subtitle_entry = Manifest::Entry{.source = std::move(subtitle.path), .type = subtitle.type};
			subtitle_entry.destination = m_subtitle_formatter.format_path(subtitle_directory, subtitle_entry.source.extension().string());
			ret.entries.push_back(std::move(subtitle_entry));
		}
	}

	return ret;
}
} // namespace vifo
