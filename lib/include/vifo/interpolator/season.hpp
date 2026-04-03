#pragma once
#include "vifo/environment.hpp"
#include "vifo/expression.hpp"
#include "vifo/interpolator/video.hpp"
#include "vifo/omdb.hpp"
#include "vifo/types.hpp"

namespace vifo::interpolator {
struct SeasonFormat {
	std::string_view video{"{series_title} {episode_id} - {episode_title}"};
	std::string_view video_fallback{"{series_title} {episode_id}"};
	std::string_view directory{"{series_title} - {season_id}"};
};

class Season : public IVideo {
  public:
	static constexpr std::string_view series_title_identifier_v{"series_title"};
	static constexpr std::string_view episode_title_identifier_v{"episode_title"};
	static constexpr std::string_view season_id_identifier_v{"season_id"};
	static constexpr std::string_view episode_id_identifier_v{"episode_id"};

	[[nodiscard]] static auto create(SeasonFormat const& format = {}) -> Result<Season>;

	void set_season(omdb::Season season);
	[[nodiscard]] auto interpolate_video(fs::path const& video) -> fs::path final;

  private:
	[[nodiscard]] auto set_episode(EpisodeId episode_id) -> expression::Expression const&;

	expression::Expression m_video{};
	std::optional<expression::Expression> m_video_fallback{};
	expression::Expression m_directory{};

	omdb::Season m_season{};
	Environment m_environment{};
};
} // namespace vifo::interpolator
