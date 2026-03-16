#include "vifo/directory/visitor.hpp"
#include <filesystem>

namespace vifo::directory {
void Visitor::visit(fs::path const& root) {
	if (!fs::is_directory(root)) { return; }
	visit_subdirectory(root, {});
}

void Visitor::visit_subdirectory(fs::path const& root, fs::path const& subdir, int const depth) {
	for (auto const& it : fs::directory_iterator{root / subdir}) {
		auto const path = fs::relative(it.path(), root);
		if (it.is_regular_file()) {
			accept_file(path);
			continue;
		}

		if (it.is_directory()) {
			if (depth >= max_depth) { continue; }
			if (!accept_directory(path)) { continue; }
			visit_subdirectory(root, path, depth + 1);
		}
	}
}
} // namespace vifo::directory
