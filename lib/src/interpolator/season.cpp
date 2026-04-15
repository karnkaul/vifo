#include "vifo/interpolator/season.hpp"
#include "vifo/expression.hpp"
#include "vifo/omdb.hpp"
#include "vifo/util/util.hpp"
#include <algorithm>
#include <string_view>

namespace vifo::interpolator {
namespace {
[[nodiscard]] auto parse_if_nonempty(std::string_view const input) -> Result<std::optional<expression::Expression>> {
	if (input.empty()) { return std::nullopt; }
	return expression::parse(input);
}
} // namespace

auto Season::create(SeasonFormat const& format) -> Result<Season> {
	auto ret = Season{};
	return expression::parse(format.video)
		.and_then([&](expression::Expression video) {
			ret.m_video = std::move(video);
			return expression::parse(format.directory);
		})
		.and_then([&](expression::Expression season_directory) {
			ret.m_directory = std::move(season_directory);
			return parse_if_nonempty(format.video_fallback);
		})
		.transform([&](std::optional<expression::Expression> episode_fallback) {
			ret.m_video_fallback = std::move(episode_fallback);
			return std::move(ret);
		});
}

void Season::set_season(omdb::Season season) {
	m_season = std::move(season);
	m_environment.set_symbol(series_title_identifier_v, m_season.title);
	m_environment.set_symbol(season_id_identifier_v, SeasonId{.number = m_season.number}.format());
}

auto Season::interpolate_video(fs::path const& video) -> fs::path {
	auto const episode_id = util::extract_episode_id(video.stem().string());
	if (!episode_id) { return {}; }

	m_environment.set_symbol(episode_id_identifier_v, episode_id->format());

	auto const& video_expression = set_episode(*episode_id);

	auto ret = m_environment.interpolate(video_expression);
	ret += video.extension().string();
	if (!video.has_parent_path()) { return ret; }

	auto season_directory = video.parent_path();
	season_directory.replace_filename(m_environment.interpolate(m_directory));
	return season_directory / ret;
}

auto Season::set_episode(EpisodeId const episode_id) -> expression::Expression const& {
	auto const it = std::ranges::find_if(m_season.episodes, [episode_id](omdb::Episode const& e) { return e.number == episode_id.number; });
	if (it == m_season.episodes.end()) { return m_video_fallback ? *m_video_fallback : m_video; }

	auto const& episode = *it;
	m_environment.set_symbol(episode_title_identifier_v, episode.title);
	return m_video;
}
} // namespace vifo::interpolator
