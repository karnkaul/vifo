#include "vifo/interpolator/season.hpp"
#include "vifo/expression.hpp"
#include "vifo/metadata.hpp"
#include "vifo/omdb.hpp"
#include "vifo/util/util.hpp"
#include <algorithm>

namespace vifo::interpolator {
auto Season::create(Format format) -> Result<Season> {
	auto ret = Season{};
	util::ensure_nonempty(format);
	return expression::parse(format.titled_video)
		.and_then([&](expression::Expression video) {
			ret.m_titled_video = std::move(video);
			return expression::parse(format.directory);
		})
		.and_then([&](expression::Expression season_directory) {
			ret.m_directory = std::move(season_directory);
			return expression::parse(format.untitled_video);
		})
		.transform([&](expression::Expression episode_fallback) {
			ret.m_untitled_video = std::move(episode_fallback);
			return std::move(ret);
		});
}

void Season::set_season(omdb::Season season) {
	m_season = std::move(season);
	m_environment.set_symbol(series_title_identifier_v, m_season.title);
	m_environment.set_symbol(season_id_identifier_v, SeasonId{.number = m_season.number}.format());
}

auto Season::interpolate_video(fs::path const& video) -> fs::path {
	auto const metadata = Metadata::build(video.stem().generic_string());
	if (!metadata.episode_id) { return {}; }

	auto const& video_expression = get_video_expression_for(*metadata.episode_id);
	return util::interpolate_video(video, m_environment, m_directory, video_expression);
}

auto Season::get_video_expression_for(EpisodeId const episode_id) -> expression::Expression const& {
	m_environment.set_symbol(episode_id_identifier_v, episode_id.format());

	auto const it = std::ranges::find_if(m_season.episodes, [episode_id](omdb::Episode const& e) { return e.number == episode_id.number; });
	if (it == m_season.episodes.end()) { return m_untitled_video; }

	auto const& episode = *it;

	if (!episode.title.empty()) {
		m_environment.set_symbol(episode_title_identifier_v, episode.title);
		return m_titled_video;
	}

	return m_untitled_video;
}
} // namespace vifo::interpolator
