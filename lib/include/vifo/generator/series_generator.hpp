#pragma once
#include "vifo/formatter/title_formatter.hpp"
#include "vifo/generator/omdb_generator.hpp"
#include "vifo/omdb.hpp"

namespace vifo {
struct SeriesGeneratorFormat {
	TitleFormat directory{};
};

class SeriesGenerator : public IOmdbGenerator {
  public:
	using IOmdbGenerator::IOmdbGenerator;

	using Format = SeriesGeneratorFormat;

	[[nodiscard]] static auto create(omdb::IService const& omdb_service, Format const& format = {}) -> Result<SeriesGenerator>;

	[[nodiscard]] auto generate_manifest(fs::path const& directory) -> Result<Manifest> final;

  private:
	TitleFormatter m_formatter{};
};
} // namespace vifo
