#pragma once
#include "djson/json.hpp"
#include "vifo/formatter/movie.hpp"
#include "vifo/formatter/pattern_swap.hpp"
#include "vifo/formatter/season.hpp"
#include "vifo/formatter/series.hpp"
#include "vifo/interpolator/season.hpp"
#include "vifo/interpolator/subtitle.hpp"
#include "vifo/interpolator/title.hpp"
#include "vifo/omdb.hpp"

namespace vifo {
void from_json(dj::Json const& json, omdb::Movie& out);
void from_json(dj::Json const& json, omdb::Episode& out);
void from_json(dj::Json const& json, omdb::Season& out);
void from_json(dj::Json const& json, omdb::Series& out);

// assigned views reference strings in json, it must outlive objects.
void from_json(dj::Json const& json, interpolator::PatternSwap::Format& format);
void to_json(dj::Json& json, interpolator::PatternSwap::Format const& format);
void from_json(dj::Json const& json, formatter::PatternSwap::Format& format);
void to_json(dj::Json& json, formatter::PatternSwap::Format const& format);

void from_json(dj::Json const& json, interpolator::TitleFormat& format);
void to_json(dj::Json& json, interpolator::TitleFormat const& format);
void from_json(dj::Json const& json, interpolator::SubtitleFormat& format);
void to_json(dj::Json& json, interpolator::SubtitleFormat const& format);
void from_json(dj::Json const& json, formatter::Movie::Format& format);
void to_json(dj::Json& json, formatter::Movie::Format const& format);

void from_json(dj::Json const& json, interpolator::SeasonFormat& format);
void to_json(dj::Json& json, interpolator::SeasonFormat const& format);
void from_json(dj::Json const& json, formatter::Season::Format& format);
void to_json(dj::Json& json, formatter::Season::Format const& format);

void from_json(dj::Json const& json, formatter::Series::Format& format);
void to_json(dj::Json& json, formatter::Series::Format const& format);
} // namespace vifo
