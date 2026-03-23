#pragma once
#include "vifo/media_file.hpp"
#include <filesystem>
#include <string>
#include <vector>

namespace vifo::detail {
struct MediaDirectory {
	[[nodiscard]] static auto scan_directory(fs::path path) -> MediaDirectory;

	[[nodiscard]] auto format_table() const -> std::string;

	fs::path path{};
	std::vector<MediaFile> files{};
};
} // namespace vifo::detail
