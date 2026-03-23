#pragma once
#include "vifo/formatter/title_formatter.hpp"
#include "vifo/generator/video_generator.hpp"
#include "vifo/omdb.hpp"

namespace vifo {
struct MovieGeneratorFormat {
	TitleFormat movie{};
	SubtitleFormat subtitle{};
};

class MovieGenerator : public VideoGenerator {
  public:
	using VideoGenerator::VideoGenerator;

	using Format = MovieGeneratorFormat;

	[[nodiscard]] static auto create(omdb::IService const& omdb_service, Format const& format = {}) -> Result<MovieGenerator>;

	[[nodiscard]] auto generate_manifest(fs::path const& directory) -> Result<Manifest> final;

  private:
	TitleFormatter m_movie_formatter{};
};
} // namespace vifo
