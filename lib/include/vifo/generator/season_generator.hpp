#pragma once
#include "vifo/formatter/season_formatter.hpp"
#include "vifo/generator/video_generator.hpp"
#include "vifo/omdb.hpp"

namespace vifo {
struct SeasonGeneratorFormat {
	SeasonFormat season{};
	SubtitleFormat subtitle{};
};

class SeasonGenerator : public VideoGenerator {
  public:
	using VideoGenerator::VideoGenerator;

	using Format = SeasonGeneratorFormat;

	[[nodiscard]] static auto create(omdb::IService const& omdb_service, Format const& format = {}) -> Result<SeasonGenerator>;

	[[nodiscard]] auto generate_manifest(fs::path const& directory) -> Result<Manifest> final;

  private:
	SeasonFormatter m_season_formatter{};
};
} // namespace vifo
