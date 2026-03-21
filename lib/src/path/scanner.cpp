#include "vifo/path/scanner.hpp"
#include "klib/debug/assert.hpp"
#include <system_error>

namespace vifo::path {
void Visitor::visit_path(fs::path path) {
	if (!fs::is_directory(path) && !fs::is_regular_file(path)) { return; }

	path = fs::canonical(fs::absolute(path));
	if (fs::is_directory(path)) {
		iterate(path, 0);
		accept_directory(path);
	} else {
		accept_file(path);
	}
}

void Visitor::iterate(fs::path const& directory, int const current_depth) {
	KLIB_ASSERT(fs::is_directory(directory));
	auto err = std::error_code{};
	for (auto const& it : fs::directory_iterator{directory, err}) {
		if (it.is_symlink()) { continue; }
		if (!it.is_directory() && !it.is_regular_file()) { continue; }

		if (it.is_directory()) {
			if (current_depth >= max_depth) { continue; }
			if (should_iterate(it.path())) { iterate(it.path(), current_depth + 1); }
			accept_directory(it.path());
		} else {
			accept_file(it.path());
		}
	}
}

auto ListScanner::scan_paths(fs::path path) -> List {
	visit_path(path);
	m_ret.scan_path = std::move(path);
	return std::move(m_ret);
}

void ListScanner::store(fs::path path) {
	if (!should_store(path)) { return; }
	m_ret.paths.push_back(std::move(path));
}
} // namespace vifo::path
