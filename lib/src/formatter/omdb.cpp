#include "vifo/formatter/omdb.hpp"

namespace vifo::formatter {
auto Omdb::get_search_title(fs::path const& path) const -> std::string { return std::move(get_metadata(path).title); }

auto Omdb::get_metadata(fs::path const& path) const -> Metadata {
	auto ret = Metadata::build(path);
	if (!search_title_override.empty()) { ret.title = search_title_override; }
	return ret;
}
} // namespace vifo::formatter
