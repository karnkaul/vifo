#pragma once
#include "vifo/types.hpp"
#include <filesystem>

namespace vifo {
namespace fs = std::filesystem;

struct Metadata {
	[[nodiscard]] static auto build(fs::path const& path) -> Metadata;

	[[nodiscard]] auto to_table() const -> std::string;

	std::string title{};
	std::string subtitle{};
	std::optional<SeasonId> season_id{};
	std::optional<EpisodeId> episode_id{};
	std::string year{};
	std::string resolution{};
};
} // namespace vifo
