#pragma once
#include "vifo/formatter/video.hpp"
#include "vifo/interpolator/season.hpp"
#include "vifo/omdb.hpp"

namespace vifo::formatter {
class Season : public Video {
  public:
	using Video::Video;

	using Format = SeasonFormat;

	[[nodiscard]] static auto create(omdb::IService const& omdb_service, Format const& format = {}) -> Result<Season>;

	[[nodiscard]] auto generate_manifest(fs::path const& directory) -> Result<Manifest> final;

  private:
	interpolator::Season m_season{};
};
} // namespace vifo::formatter
