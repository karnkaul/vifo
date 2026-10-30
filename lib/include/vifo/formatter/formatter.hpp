#pragma once
#include "klib/base_types.hpp"
#include "vifo/manifest.hpp"
#include "vifo/result.hpp"
#include <filesystem>

namespace vifo {
namespace fs = std::filesystem;

class IFormatter : public klib::Polymorphic {
  public:
	[[nodiscard]] auto generate_manifest(fs::path const& directory) -> Result<Manifest>;

  protected:
	virtual auto do_generate_manifest(fs::path const& directory) -> Result<Manifest> = 0;
};
} // namespace vifo
