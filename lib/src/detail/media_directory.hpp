#pragma once
#include "vifo/media_file.hpp"
#include <filesystem>
#include <string>
#include <vector>

namespace vifo::detail {
struct MediaDirectory {
	static constexpr auto max_depth_v{10};

	[[nodiscard]] static auto scan_directory(fs::path path, int max_depth = max_depth_v) -> MediaDirectory;

	[[nodiscard]] auto format_table() const -> std::string;

	fs::path path{};
	std::vector<MediaFile> files{};
};
} // namespace vifo::detail
