#include "vifo/formatter/formatter.hpp"
#include "detail/common.hpp"

namespace vifo {
auto IFormatter::generate_manifest(fs::path const& directory) -> Result<Manifest> {
	return do_generate_manifest(directory).transform([](Manifest ret) {
		detail::ensure_relative_paths(ret);
		return ret;
	});
}
} // namespace vifo
