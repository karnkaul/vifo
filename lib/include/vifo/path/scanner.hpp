#pragma once
#include "klib/base_types.hpp"
#include "vifo/path/list.hpp"

namespace vifo::path {
class Visitor : public klib::Polymorphic {
  public:
	virtual void visit_path(fs::path path);

	int max_depth{10};

  protected:
	virtual void accept_directory(fs::path const& path) = 0;
	virtual void accept_file(fs::path const& path) = 0;

	[[nodiscard]] virtual auto should_iterate([[maybe_unused]] fs::path const& directory) const -> bool { return true; }

  private:
	void iterate(fs::path const& directory, int current_depth);
};

class ListScanner : protected Visitor {
  public:
	[[nodiscard]] virtual auto scan_paths(fs::path path) -> List;

	int max_depth{10};

  protected:
	[[nodiscard]] virtual auto should_store([[maybe_unused]] fs::path const& path) const -> bool { return true; }

  private:
	void accept_directory(fs::path const& path) final { store(path); }
	void accept_file(fs::path const& path) final { store(path); }

	void store(fs::path path);

	List m_ret{};
};
} // namespace vifo::path
