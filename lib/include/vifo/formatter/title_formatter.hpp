#pragma once
#include "vifo/environment.hpp"
#include "vifo/expression.hpp"
#include <filesystem>

namespace vifo {
namespace fs = std::filesystem;

struct TitleFormat {
	std::string_view video{"{title}"};
	std::string_view directory{"({year}) {title}"};
};

class TitleFormatter {
  public:
	static constexpr std::string_view title_identifier_v{"title"};
	static constexpr std::string_view year_identifier_v{"year"};

	using Format = TitleFormat;

	[[nodiscard]] static auto create(Format const& format = {}) -> Result<TitleFormatter>;

	void set_title(std::string title);
	void set_year(int year);

	[[nodiscard]] auto format_dirname() const -> std::string;
	[[nodiscard]] auto format_directory(fs::path const& source_directory) const -> fs::path;
	[[nodiscard]] auto format_video(fs::path const& video) const -> fs::path;

  private:
	expression::Expression m_video{};
	expression::Expression m_directory{};

	Environment m_environment{};
};
} // namespace vifo
