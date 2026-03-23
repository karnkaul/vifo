#pragma once
#include "klib/base_types.hpp"
#include <filesystem>

namespace vifo::interpolator {
namespace fs = std::filesystem;

class IVideo : public klib::Polymorphic {
  public:
	[[nodiscard]] virtual auto interpolate_video(fs::path const& video) -> fs::path = 0;
};
} // namespace vifo::interpolator
