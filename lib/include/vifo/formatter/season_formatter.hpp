#pragma once
#include "vifo/expression.hpp"
#include "vifo/formatter/omdb_formatter.hpp"
#include "vifo/omdb.hpp"

namespace vifo {
struct SeasonFormat {
	std::string_view video{"{series_title} {episode_id} - {episode_title}"};
	std::string_view directory{"{series_title} - {season_id}"};
};

class SeasonFormatter : public OmdbFormatter {
  public:
	[[nodiscard]] static auto create(SeasonFormat const& format = {}) -> Result<SeasonFormatter>;

	void set_season(omdb::Season season);
	[[nodiscard]] auto format_path(fs::path const& video) -> fs::path;

  private:
	expression::Expression m_video{};
	expression::Expression m_directory{};

	omdb::Season m_season{};
};
} // namespace vifo
