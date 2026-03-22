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
	static constexpr std::string_view year_identifier_v{"year"};

	[[nodiscard]] auto get_subtitles_dir_for(fs::path const& media_file) const -> fs::path;

	std::string subtitles_dirname{"subs"};

  protected:
	Environment m_environment{};
};
} // namespace vifo
