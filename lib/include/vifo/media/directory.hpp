#pragma once
#include "vifo/media/file.hpp"
#include <filesystem>
#include <string>
#include <vector>

namespace vifo {
struct MediaDirectory {
	[[nodiscard]] static auto scan_directory(fs::path path) -> MediaDirectory;

	[[nodiscard]] auto format_table() const -> std::string;

	fs::path path{};
	std::vector<MediaFile> files{};
};
} // namespace vifo
