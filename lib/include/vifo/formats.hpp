#pragma once
#include <optional>
#include <string_view>

namespace vifo {
namespace interpolator {
struct PatternSwapFormat {
	std::string_view input{};
	std::string_view output{};
};

struct TitleFormat {
	std::string_view video{"{title}"};
	std::string_view directory{"{title} ({year})"};
};

struct SubtitleFormat {
	std::string_view output{"{title}.en.sub_{number}"};
};

struct SeasonFormat {
	std::string_view titled_video{"{series_title} {episode_id} - {episode_title}"};
	std::string_view untitled_video{"{series_title} {episode_id}"};
	std::string_view directory{"{series_title} - {season_id}"};
};
} // namespace interpolator

namespace formatter {
struct PatternSwapFormat {
	interpolator::PatternSwapFormat directory{};
	std::optional<interpolator::PatternSwapFormat> file{};
};

struct MovieFormat {
	interpolator::TitleFormat movie{};
	interpolator::SubtitleFormat subtitle{};
};

struct SeasonFormat {
	interpolator::SeasonFormat season{};
	interpolator::SubtitleFormat subtitle{};
};

struct SeriesFormat {
	interpolator::TitleFormat directory{};
	SeasonFormat season{};
};
} // namespace formatter

struct Formats {
	[[nodiscard]] constexpr auto to_movie_format() const -> formatter::MovieFormat { return {.movie = movie, .subtitle = subtitle}; }
	[[nodiscard]] constexpr auto to_season_format() const -> formatter::SeasonFormat { return {.season = season, .subtitle = subtitle}; }
	[[nodiscard]] constexpr auto to_series_format() const -> formatter::SeriesFormat { return {.directory = series, .season = to_season_format()}; }

	formatter::PatternSwapFormat pattern_swap{};
	interpolator::TitleFormat movie{};
	interpolator::TitleFormat series{};
	interpolator::SeasonFormat season{};
	interpolator::SubtitleFormat subtitle{};
};
} // namespace vifo
