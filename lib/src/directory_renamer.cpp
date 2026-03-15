#include "vifo/directory_renamer.hpp"
#include "vifo/formatter.hpp"
#include "vifo/util.hpp"
#include <filesystem>

namespace vifo {
auto DirectoryRenamer::create(std::string_view const input_expression, std::string_view const output_expression) -> Result<DirectoryRenamer> {
	return create_interpolator(input_expression, output_expression).transform([](std::unique_ptr<IFormatter> interpolator) {
		auto ret = DirectoryRenamer{};
		ret.m_interpolator = std::move(interpolator);
		return ret;
	});
}

auto DirectoryRenamer::build_manifest(fs::path root) const -> Manifest {
	if (!fs::is_directory(root)) { return {}; }

	auto ret = Manifest{.root = std::move(root)};
	for (auto const& it : fs::recursive_directory_iterator{ret.root}) {
		if (!it.is_directory()) { continue; }

		auto const src_filename = it.path().filename().string();
		auto const dst_filename = m_interpolator->format(src_filename);
		if (dst_filename.empty()) { continue; }

		ret.entries.push_back(Manifest::Entry{
			.source = fs::relative(it.path(), ret.root),
			.destination = fs::relative(util::prefix_parent(it.path(), dst_filename), ret.root),
		});
	}
	return ret;
}
} // namespace vifo
