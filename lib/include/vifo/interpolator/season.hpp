#pragma once
#include "vifo/environment.hpp"
#include "vifo/expression.hpp"
#include "vifo/formats.hpp"
#include "vifo/interpolator/video.hpp"
#include "vifo/omdb.hpp"
#include "vifo/types.hpp"

namespace vifo::interpolator {
class Season : public IVideo {
  public:
	using Format = SeasonFormat;

	static constexpr std::string_view series_title_identifier_v{"series_title"};
	static constexpr std::string_view episode_title_identifier_v{"episode_title"};
	static constexpr std::string_view season_id_identifier_v{"season_id"};
	static constexpr std::string_view episode_id_identifier_v{"episode_id"};

	[[nodiscard]] static auto create(Format format = {}) -> Result<Season>;

	void set_season(omdb::Season season);
	[[nodiscard]] auto interpolate_video(fs::path const& video) -> fs::path final;

  private:
	[[nodiscard]] auto get_video_expression_for(EpisodeId episode_id) -> expression::Expression const&;

	expression::Expression m_titled_video{};
	expression::Expression m_untitled_video{};
	expression::Expression m_directory{};

	omdb::Season m_season{};
	Environment m_environment{};
};
} // namespace vifo::interpolator
