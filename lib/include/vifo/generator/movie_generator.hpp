#pragma once
#include "vifo/formatter/subtitle_formatter.hpp"
#include "vifo/formatter/title_formatter.hpp"
#include "vifo/generator/omdb_generator.hpp"
#include "vifo/omdb.hpp"

namespace vifo {
struct MovieGeneratorFormat {
	TitleFormat movie{};
	SubtitleFormat subtitle{};
};

class MovieGenerator : public IOmdbGenerator {
  public:
	using IOmdbGenerator::IOmdbGenerator;

	using Format = MovieGeneratorFormat;

	[[nodiscard]] static auto create(omdb::IService const& omdb_service, Format const& format = {}) -> Result<MovieGenerator>;

	[[nodiscard]] auto generate_manifest(fs::path const& directory) -> Result<Manifest> final;

  private:
	TitleFormatter m_movie_formatter{};
	SubtitleFormatter m_subtitle_formatter{};
};
} // namespace vifo
