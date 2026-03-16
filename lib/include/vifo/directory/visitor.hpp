#pragma once
#include "klib/base_types.hpp"
#include <filesystem>

namespace vifo::directory {
namespace fs = std::filesystem;

class Visitor : public klib::Polymorphic {
  public:
	virtual auto accept_directory(fs::path const& /*path*/) -> bool { return true; }
	virtual void accept_file(fs::path const& /*path*/) {}

	virtual void visit(fs::path const& root);

	int max_depth{10};

  private:
	void visit_subdirectory(fs::path const& root, fs::path const& subdir, int depth = 0);
};
} // namespace vifo::directory
