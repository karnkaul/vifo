#include "vifo/types.hpp"
#include <format>

namespace vifo {
auto SeasonId::format() const -> std::string { return std::format("S{:02}", number); }

auto EpisodeId::format() const -> std::string { return std::format("S{:02}E{:02}", season, number); }
} // namespace vifo
