#include "vifo/formatter/pattern_swap.hpp"
#include "detail/common.hpp"
#include "vifo/interpolator/pattern_swap.hpp"
#include "vifo/path/scanner.hpp"
#include "vifo/types.hpp"
#include "vifo/util/util.hpp"
#include <filesystem>

namespace vifo::formatter {
namespace {
class Scanner : public path::ListScanner {
  public:
	bool list_files{};

  private:
	[[nodiscard]] auto should_store([[maybe_unused]] fs::path const& path) const -> bool final { return list_files || fs::is_directory(path); }
};

[[nodiscard]] auto format_path(interpolator::PatternSwap& interpolator, fs::path const& source) -> fs::path {
	auto const source_stem = source.stem().string();
	auto const destination_stem = interpolator.interpolate(source_stem);
	if (destination_stem.empty()) { return {}; }

	auto ret = util::prefix_parent(source, destination_stem);
	ret += source.extension();
	return ret;
}
} // namespace

auto PatternSwap::create(Format const& format) -> Result<PatternSwap> {
	auto ret = PatternSwap{};
	return ret.create_directory(format.directory).and_then([&] { return ret.create_file(format.file); }).transform([&] { return std::move(ret); });
}

auto PatternSwap::do_generate_manifest(fs::path const& directory) -> Result<Manifest> {
	if (!fs::is_directory(directory)) { return detail::to_error(Error::Type::Argument, std::format("not a directory: '{}'", directory.generic_string())); }

	auto scanner = Scanner{};
	scanner.list_files = m_file.has_value();
	auto path_list = scanner.scan_paths(directory, max_depth);

	if (path_list.paths.empty()) { return detail::to_error(Error::Type::Identify, std::format("no entries found in: '{}'", directory.generic_string())); }

	auto ret = Manifest{.parent = std::move(path_list.scan_path)};
	for (auto& source : path_list.paths) {
		if (source.empty()) { continue; }

		auto source_relative = util::to_relative(ret.parent, source);
		auto const type = fs::is_directory(source) ? MediaFileType::Directory : MediaFileType::Unknown;
		auto destination = [&] {
			if (fs::is_regular_file(source)) { return format_path(*m_file, source_relative); }
			return format_path(m_directory, source_relative);
		}();
		if (destination.empty()) {
			ret.orphans.push_back(Manifest::Entry{.source = std::move(source_relative), .type = type});
			continue;
		}
		if (destination == source_relative) { continue; }

		if (source == ret.parent) { ret.parent = ret.parent.parent_path(); }
		ret.entries.push_back(Manifest::Entry{.source = std::move(source_relative), .destination = std::move(destination), .type = type});
	}
	return ret;
}

auto PatternSwap::create_directory(interpolator::PatternSwapFormat const& format) -> Result<void> {
	return interpolator::PatternSwap::create(format).transform([&](interpolator::PatternSwap swapper) { m_directory = std::move(swapper); });
}

auto PatternSwap::create_file(std::optional<interpolator::PatternSwapFormat> const& format) -> Result<void> {
	if (!format) { return {}; }
	return interpolator::PatternSwap::create(*format).transform([&](interpolator::PatternSwap swapper) { m_file = std::move(swapper); });
}
} // namespace vifo::formatter
