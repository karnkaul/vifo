#pragma once
#include "vifo/expression.hpp"
#include "vifo/formatter/omdb_formatter.hpp"
#include "vifo/omdb.hpp"

namespace vifo {
struct MovieFormat {
	std::string_view video{"{title}"};
	std::string_view directory{"({year}) {title}"};
};

class MovieFormatter : public OmdbFormatter {
  public:
	[[nodiscard]] static auto create(MovieFormat const& format = {}) -> Result<MovieFormatter>;

	void set_movie(omdb::Movie movie);
	[[nodiscard]] auto format_path(fs::path const& video) const -> fs::path;

  private:
	expression::Expression m_video{};
	expression::Expression m_directory{};
};
} // namespace vifo
