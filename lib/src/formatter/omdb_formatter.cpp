#include "vifo/formatter/omdb_formatter.hpp"
#include "vifo/util/util.hpp"

namespace vifo {
auto OmdbFormatter::get_subtitles_dir_for(fs::path const& media_file) const -> fs::path { return util::prefix_parent(media_file, subtitles_dirname); }
} // namespace vifo
