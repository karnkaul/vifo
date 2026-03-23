#include "vifo/media/directory.hpp"
#include "vifo/path/scanner.hpp"
#include "vifo/types.hpp"
#include "vifo/util/util.hpp"
#include <array>
#include <filesystem>

namespace vifo {
auto MediaDirectory::scan_directory(fs::path path) -> MediaDirectory {
	struct Scanner : path::Visitor {
		void accept_directory(fs::path const& /*path*/) final {}

		void accept_file(fs::path const& path) final {
			auto const type = util::get_media_file_type(path);
			if (!type) { return; }

			auto const size = std::int64_t(fs::file_size(path));
			ret.files.push_back(MediaFile{.path = path, .size = size, .type = *type});
		}

		MediaDirectory ret{};
	};

	auto scanner = Scanner{};
	scanner.visit_path(path);
	scanner.ret.path = std::move(path);
	return std::move(scanner.ret);
}

auto MediaDirectory::format_table() const -> std::string {
	if (files.empty()) { return {}; }

	static constexpr auto headers_v = std::array{"path", "type"};
	auto const per_entry = [&](std::vector<std::string>& row, MediaFile const& file) {
		row.push_back(util::to_relative(path, file.path));
		row.emplace_back(media_file_type_name_map.to_name(file.type));
	};
	return util::format_enumerated_table(headers_v, files, per_entry);
}
} // namespace vifo
