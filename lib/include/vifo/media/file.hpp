#pragma once
#include "vifo/types.hpp"
#include <cstdint>
#include <filesystem>

namespace vifo {
namespace fs = std::filesystem;

struct MediaFile {
	using Type = MediaFileType;

	fs::path path{};
	std::int64_t size{};
	Type type{};
};
} // namespace vifo
