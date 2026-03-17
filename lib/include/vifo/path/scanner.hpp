#pragma once
#include "klib/base_types.hpp"
#include <filesystem>
#include <vector>

namespace vifo::path {
namespace fs = std::filesystem;

class Scanner : public klib::Polymorphic {
  public:
	[[nodiscard]] virtual auto scan_paths(fs::path path) -> std::vector<fs::path>;

	int max_depth{10};

  protected:
	[[nodiscard]] virtual auto should_iterate([[maybe_unused]] fs::path const& directory) const -> bool { return true; }
	[[nodiscard]] virtual auto should_store([[maybe_unused]] fs::path const& path) const -> bool { return true; }

  private:
	void iterate(fs::path const& directory, int current_depth);
	void store(fs::path path);

	std::vector<fs::path> m_ret{};
};
} // namespace vifo::path
