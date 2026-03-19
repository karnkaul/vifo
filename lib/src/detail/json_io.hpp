#pragma once
#include "djson/json.hpp"
#include "vifo/omdb.hpp"

namespace vifo::detail {
void from_json(dj::Json const& json, omdb::Movie& out);
void from_json(dj::Json const& json, omdb::Episode& out);
void from_json(dj::Json const& json, omdb::Season& out);
void from_json(dj::Json const& json, omdb::Series& out);
} // namespace vifo::detail
