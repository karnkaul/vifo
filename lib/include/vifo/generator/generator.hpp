#pragma once
#include "klib/base_types.hpp"
#include "vifo/manifest.hpp"
#include <filesystem>
#include <vector>

namespace vifo {
namespace fs = std::filesystem;

struct Video {
	fs::path path{};
	std::vector<fs::path> subtitles{};
};

struct Movie {
	Video video{};
};

class IGenerator : public klib::Polymorphic {
  public:
	[[nodiscard]] virtual auto generate_manifest(fs::path const& directory) -> Manifest = 0;
};
} // namespace vifo
