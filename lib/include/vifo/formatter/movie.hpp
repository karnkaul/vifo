#pragma once
#include "vifo/formatter/video.hpp"
#include "vifo/interpolator/title.hpp"
#include "vifo/omdb.hpp"

namespace vifo::formatter {
struct MovieFormat {
	interpolator::TitleFormat movie{};
	interpolator::SubtitleFormat subtitle{};
};

class Movie : public Video {
  public:
	using Video::Video;

	using Format = MovieFormat;

	[[nodiscard]] static auto create(omdb::IService const& omdb_service, Format const& format = {}) -> Result<Movie>;

	[[nodiscard]] auto generate_manifest(fs::path const& directory) -> Result<Manifest> final;

  private:
	interpolator::Title m_movie{};
};
} // namespace vifo::formatter
