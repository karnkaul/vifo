#pragma once
#include "vifo/formatter/omdb.hpp"
#include "vifo/interpolator/title.hpp"
#include "vifo/omdb.hpp"

namespace vifo::formatter {
struct SeriesFormat {
	interpolator::TitleFormat directory{};
};

class Series : public Omdb {
  public:
	using Omdb::Omdb;

	using Format = SeriesFormat;

	[[nodiscard]] static auto create(omdb::IService const& omdb_service, Format const& format = {}) -> Result<Series>;

	[[nodiscard]] auto generate_manifest(fs::path const& directory) -> Result<Manifest> final;

  private:
	interpolator::Title m_title{};
};
} // namespace vifo::formatter
