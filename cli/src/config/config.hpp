#pragma once
#include "vifo/formats.hpp"

namespace vifo::cli {
struct Config {
	std::string_view omdb_api_token{};
	Formats formats{};
};
} // namespace vifo::cli
