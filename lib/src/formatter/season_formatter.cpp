#include "vifo/formatter/season_formatter.hpp"
#include "vifo/expression.hpp"
#include "vifo/omdb.hpp"
#include "vifo/util/util.hpp"
#include <algorithm>

namespace vifo {
auto SeasonFormatter::create(SeasonFormat const& format) -> Result<SeasonFormatter> {
	auto ret = SeasonFormatter{};
	return expression::parse(format.video)
		.and_then([&](expression::Expression video) {
			ret.m_video = std::move(video);
			return expression::parse(format.directory);
		})
		.transform([&](expression::Expression season_directory) {
			ret.m_directory = std::move(season_directory);
			return std::move(ret);
		});
}

void SeasonFormatter::set_season(omdb::Season season) {
	m_season = std::move(season);
	m_environment.set_symbol(series_title_identifier_v, m_season.title);
	m_environment.set_symbol(season_id_identifier_v, std::format("S{:02}", m_season.number));
}

auto SeasonFormatter::format_path(fs::path const& video) -> fs::path {
	auto const episode_id = util::extract_episode_id(video.stem().string());
	if (!episode_id) { return {}; }

	auto const it = std::ranges::find_if(m_season.episodes, [episode_id](omdb::Episode const& e) { return e.number == episode_id->number; });
	if (it == m_season.episodes.end()) { return {}; }

	auto const& episode = *it;
	m_environment.set_symbol(episode_id_identifier_v, std::format("S{:02}E{:02}", m_season.number, episode.number));
	m_environment.set_symbol(episode_title_identifier_v, episode.title);

	auto ret = fs::path{m_environment.interpolate(m_video)};
	ret += video.extension();
	if (!video.has_parent_path()) { return ret; }

	auto season_directory = video.parent_path();
	season_directory.replace_filename(m_environment.interpolate(m_directory));
	return season_directory / ret;
}
} // namespace vifo
