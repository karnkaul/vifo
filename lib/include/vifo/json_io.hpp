#pragma once
#include "djson/json.hpp"
#include "vifo/formatter/pattern_swap.hpp"
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
} // namespace vifo
