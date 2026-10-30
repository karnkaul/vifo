#pragma once
#include "vifo/formatter/video.hpp"
#include "vifo/interpolator/season.hpp"
#include "vifo/omdb.hpp"

namespace vifo::formatter {
class Season : public Video {
  public:
	using Format = SeasonFormat;

	[[nodiscard]] static auto create(omdb::IService const& omdb_service, Format const& format = {}) -> Result<Season>;

  private:
	using Video::Video;

	[[nodiscard]] auto do_generate_manifest(fs::path const& directory) -> Result<Manifest> final;

	interpolator::Season m_season{};
};
} // namespace vifo::formatter
