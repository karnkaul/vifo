#pragma once
#include "djson/json.hpp"
#include "vifo/formats.hpp"
#include "vifo/omdb.hpp"

namespace vifo {
void from_json(dj::Json const& json, omdb::Movie& out);
void from_json(dj::Json const& json, omdb::Episode& out);
void from_json(dj::Json const& json, omdb::Season& out);
void from_json(dj::Json const& json, omdb::Series& out);

// assigned views reference strings in json, it must outlive objects.
void from_json(dj::Json const& json, interpolator::PatternSwapFormat& format);
void to_json(dj::Json& json, interpolator::PatternSwapFormat const& format);
void from_json(dj::Json const& json, formatter::PatternSwapFormat& format);
void to_json(dj::Json& json, formatter::PatternSwapFormat const& format);

void from_json(dj::Json const& json, interpolator::TitleFormat& format);
void to_json(dj::Json& json, interpolator::TitleFormat const& format);
void from_json(dj::Json const& json, interpolator::SubtitleFormat& format);
void to_json(dj::Json& json, interpolator::SubtitleFormat const& format);
void from_json(dj::Json const& json, formatter::MovieFormat& format);
void to_json(dj::Json& json, formatter::MovieFormat const& format);

void from_json(dj::Json const& json, interpolator::SeasonFormat& format);
void to_json(dj::Json& json, interpolator::SeasonFormat const& format);
void from_json(dj::Json const& json, formatter::SeasonFormat& format);
void to_json(dj::Json& json, formatter::SeasonFormat const& format);

void from_json(dj::Json const& json, formatter::SeriesFormat& format);
void to_json(dj::Json& json, formatter::SeriesFormat const& format);

void from_json(dj::Json const& json, Formats& formats);
void to_json(dj::Json& json, Formats const& formats);
} // namespace vifo
