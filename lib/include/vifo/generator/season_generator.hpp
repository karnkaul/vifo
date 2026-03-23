#pragma once
#include "vifo/formatter/season_formatter.hpp"
#include "vifo/formatter/subtitle_formatter.hpp"
#include "vifo/generator/omdb_generator.hpp"
#include "vifo/omdb.hpp"

namespace vifo {
struct SeasonGeneratorFormat {
	SeasonFormat season{};
	SubtitleFormat subtitle{};
};

class SeasonGenerator : public IOmdbGenerator {
  public:
	using IOmdbGenerator::IOmdbGenerator;

	using Format = SeasonGeneratorFormat;

	[[nodiscard]] static auto create(omdb::IService const& omdb_service, Format const& format = {}) -> Result<SeasonGenerator>;

	[[nodiscard]] auto generate_manifest(fs::path const& directory) -> Result<Manifest> final;

  private:
	SeasonFormatter m_season_formatter{};
	SubtitleFormatter m_subtitle_formatter{};
};
} // namespace vifo
