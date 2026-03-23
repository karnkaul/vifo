#pragma once
#include "klib/base_types.hpp"
#include "vifo/environment.hpp"
#include <filesystem>
#include <string>
#include <string_view>

namespace vifo {
namespace fs = std::filesystem;

class OmdbFormatter : public klib::Polymorphic {
  public:
	static constexpr std::string_view title_identifier_v{"title"};
	static constexpr std::string_view series_title_identifier_v{"series_title"};
	static constexpr std::string_view episode_title_identifier_v{"episode_title"};
	static constexpr std::string_view year_identifier_v{"year"};
	static constexpr std::string_view season_id_identifier_v{"season_id"};
	static constexpr std::string_view episode_id_identifier_v{"episode_id"};

	[[nodiscard]] auto get_subtitles_dir_for(fs::path const& media_file) const -> fs::path;

	std::string subtitles_dirname{"subs"};

  protected:
	Environment m_environment{};
};
} // namespace vifo
