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
	std::string_view video{"{series_title} {episode_id} - {episode_title}"};
	std::string_view video_fallback{"{series_title} {episode_id}"};
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
};
} // namespace formatter

struct Formats {
	formatter::PatternSwapFormat pattern_swap{};
	formatter::MovieFormat movie{};
	formatter::SeasonFormat season{};
	formatter::SeriesFormat series{};
};
} // namespace vifo
