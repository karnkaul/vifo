#pragma once
#include "klib/base_types.hpp"
#include "vifo/path/list.hpp"

namespace vifo::path {
class Visitor : public klib::Polymorphic {
  public:
	static constexpr auto max_depth_v{10};

	virtual void visit_path(fs::path path, int max_depth = max_depth_v);

  protected:
	virtual void accept_directory(fs::path const& path) = 0;
	virtual void accept_file(fs::path const& path) = 0;

	[[nodiscard]] virtual auto should_iterate([[maybe_unused]] fs::path const& directory) const -> bool { return true; }

  private:
	void iterate(fs::path const& directory, int current_depth);

	int m_max_depth{max_depth_v};
};

class ListScanner : protected Visitor {
  public:
	[[nodiscard]] virtual auto scan_paths(fs::path path, int max_depth = max_depth_v) -> List;

  protected:
	[[nodiscard]] virtual auto should_store([[maybe_unused]] fs::path const& path) const -> bool { return true; }

  private:
	void accept_directory(fs::path const& path) final { store(path); }
	void accept_file(fs::path const& path) final { store(path); }

	void store(fs::path path);

	List m_ret{};
};
} // namespace vifo::path
