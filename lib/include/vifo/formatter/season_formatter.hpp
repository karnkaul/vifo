#pragma once
#include "vifo/environment.hpp"
#include "vifo/expression.hpp"
#include "vifo/omdb.hpp"
#include <filesystem>

namespace vifo {
namespace fs = std::filesystem;

struct SeasonFormat {
	std::string_view video{"{series_title} {episode_id} - {episode_title}"};
	std::string_view directory{"{series_title} - {season_id}"};
};

class SeasonFormatter {
  public:
	static constexpr std::string_view series_title_identifier_v{"series_title"};
	static constexpr std::string_view episode_title_identifier_v{"episode_title"};
	static constexpr std::string_view season_id_identifier_v{"season_id"};
	static constexpr std::string_view episode_id_identifier_v{"episode_id"};

	[[nodiscard]] static auto create(SeasonFormat const& format = {}) -> Result<SeasonFormatter>;

	void set_season(omdb::Season season);
	[[nodiscard]] auto format_path(fs::path const& video) -> fs::path;

  private:
	expression::Expression m_video{};
	expression::Expression m_directory{};

	omdb::Season m_season{};
	Environment m_environment{};
};
} // namespace vifo
