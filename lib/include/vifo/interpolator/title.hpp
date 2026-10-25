#pragma once
#include "vifo/environment.hpp"
#include "vifo/expression.hpp"
#include "vifo/formats.hpp"
#include "vifo/interpolator/video.hpp"

namespace vifo::interpolator {
class Title : public IVideo {
  public:
	using Format = TitleFormat;

	static constexpr std::string_view title_identifier_v{"title"};
	static constexpr std::string_view year_identifier_v{"year"};

	[[nodiscard]] static auto create(Format const& format = {}) -> Result<Title>;

	void set_title(std::string title);
	void set_year(int year);

	[[nodiscard]] auto format_dirname() const -> std::string;
	[[nodiscard]] auto format_directory(fs::path const& source_directory) const -> fs::path;
	[[nodiscard]] auto interpolate_video(fs::path const& video) -> fs::path final;

  private:
	expression::Expression m_video{};
	expression::Expression m_directory{};

	Environment m_environment{};
};
} // namespace vifo::interpolator
