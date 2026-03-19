#pragma once
#include "klib/base_types.hpp"
#include "vifo/path/list.hpp"

namespace vifo::path {
class Scanner : public klib::Polymorphic {
  public:
	[[nodiscard]] virtual auto scan_paths(fs::path path) -> List;

	int max_depth{10};

  protected:
	[[nodiscard]] virtual auto should_iterate([[maybe_unused]] fs::path const& directory) const -> bool { return true; }
	[[nodiscard]] virtual auto should_store([[maybe_unused]] fs::path const& path) const -> bool { return true; }

  private:
	void iterate(fs::path const& directory, int current_depth);
	void store(fs::path path);

	List m_ret{};
};
} // namespace vifo::path
