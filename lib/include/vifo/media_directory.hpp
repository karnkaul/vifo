#pragma once
#include "vifo/types.hpp"
#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

namespace vifo {
namespace fs = std::filesystem;

struct MediaFile {
	using Type = MediaFileType;

	fs::path path{};
	std::int64_t size{};
	Type type{};
};

struct MediaDirectory {
	[[nodiscard]] static auto scan_directory(fs::path path) -> MediaDirectory;

	[[nodiscard]] auto format_table() const -> std::string;

	fs::path path{};
	std::vector<MediaFile> files{};
};
} // namespace vifo
