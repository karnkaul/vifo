#include "vifo/formatter/omdb.hpp"
#include "vifo/util/util.hpp"

namespace vifo::formatter {
auto Omdb::get_search_title(fs::path const& path) const -> std::string {
	if (!search_title_override.empty()) { return search_title_override; }
	return util::identify_title(path);
}
} // namespace vifo::formatter
