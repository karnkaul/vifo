#pragma once
#include "vifo/formatter/video.hpp"
#include "vifo/interpolator/title.hpp"
#include "vifo/omdb.hpp"

namespace vifo::formatter {
class Movie : public Video {
  public:
	using Format = MovieFormat;

	[[nodiscard]] static auto create(omdb::IService const& omdb_service, Format const& format = {}) -> Result<Movie>;

  private:
	using Video::Video;

	[[nodiscard]] auto do_generate_manifest(fs::path const& directory) -> Result<Manifest> final;

	interpolator::Title m_movie{};
};
} // namespace vifo::formatter
