#include "vifo/path/scanner.hpp"
#include "klib/assert.hpp"

namespace vifo::path {
auto Scanner::scan_paths(fs::path path) -> List {
	if (!fs::is_directory(path) && !fs::is_regular_file(path)) { return {}; }

	path = fs::canonical(fs::absolute(path));
	if (fs::is_directory(path)) { iterate(path, 0); }
	store(path);

	return std::move(m_ret);
}

void Scanner::iterate(fs::path const& directory, int const current_depth) {
	KLIB_ASSERT(fs::is_directory(directory));
	for (auto const& it : fs::directory_iterator{directory}) {
		if (!it.is_directory() && !it.is_regular_file()) { continue; }

		if (it.is_directory()) {
			if (current_depth >= max_depth) { continue; }
			if (should_iterate(it.path())) { iterate(it.path(), current_depth + 1); }
		}
		store(it.path());
	}
}

void Scanner::store(fs::path path) {
	if (!should_store(path)) { return; }
	m_ret.paths.push_back(std::move(path));
}
} // namespace vifo::path
