#include "vifo/generator/omdb_generator.hpp"
#include "detail/common.hpp"
#include "vifo/types.hpp"
#include "vifo/util/util.hpp"
#include <filesystem>

namespace vifo {
auto IOmdbGenerator::get_subtitles_dir_for(fs::path const& media_file) const -> fs::path { return util::prefix_parent(media_file, subtitles_dirname); }

auto IOmdbGenerator::if_directory(fs::path const& path) -> Result<fs::path> {
	if (!fs::is_directory(path)) { return detail::to_error(Error::Type::Argument, std::format("not a directory: '{}'", path.generic_string())); }
	return path;
}
} // namespace vifo
