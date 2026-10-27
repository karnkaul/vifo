#pragma once
#include "vifo/types.hpp"
#include <cstdint>
#include <filesystem>
#include <optional>

namespace vifo {
namespace fs = std::filesystem;

struct MediaFile {
	using Type = MediaFileType;

	struct Metadata;

	fs::path path{};
	std::int64_t size{};
	Type type{};
};

struct MediaFile::Metadata {
	[[nodiscard]] static auto build(std::string filename) -> Metadata;

	[[nodiscard]] auto to_table() const -> std::string;

	std::string title{};
	std::string subtitle{};
	std::optional<SeasonId> season_id{};
	std::optional<EpisodeId> episode_id{};
	std::string year{};
	std::string resolution{};
};
} // namespace vifo
