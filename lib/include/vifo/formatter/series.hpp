#pragma once
#include "vifo/formatter/omdb.hpp"
#include "vifo/interpolator/title.hpp"
#include "vifo/omdb.hpp"

namespace vifo::formatter {
class Series : public Omdb {
  public:
	using Omdb::Omdb;

	using Format = SeriesFormat;

	[[nodiscard]] static auto create(omdb::IService const& omdb_service, Format const& format = {}) -> Result<Series>;

  private:
	[[nodiscard]] auto do_generate_manifest(fs::path const& directory) -> Result<Manifest> final;

	interpolator::Title m_title{};
};
} // namespace vifo::formatter
