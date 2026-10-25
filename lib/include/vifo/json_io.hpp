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
void from_json(dj::Json const& json, formatter::PatternSwapFormat& format);

void from_json(dj::Json const& json, interpolator::TitleFormat& format);
void from_json(dj::Json const& json, interpolator::SubtitleFormat& format);
void from_json(dj::Json const& json, formatter::MovieFormat& format);

void from_json(dj::Json const& json, interpolator::SeasonFormat& format);
void from_json(dj::Json const& json, formatter::SeasonFormat& format);

void from_json(dj::Json const& json, formatter::SeriesFormat& format);

void from_json(dj::Json const& json, Formats& formats);
} // namespace vifo
