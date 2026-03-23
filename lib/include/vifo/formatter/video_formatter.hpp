#pragma once
#include "klib/base_types.hpp"
#include <filesystem>

namespace vifo {
namespace fs = std::filesystem;

class IVideoFormatter : public klib::Polymorphic {
  public:
	[[nodiscard]] virtual auto format_video(fs::path const& video) -> fs::path = 0;
};
} // namespace vifo
