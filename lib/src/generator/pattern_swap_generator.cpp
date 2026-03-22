#include "vifo/generator/pattern_swap_generator.hpp"
#include "detail/common.hpp"
#include "vifo/formatter/pattern_swap_formatter.hpp"
#include "vifo/path/scanner.hpp"
#include "vifo/util/util.hpp"
#include <filesystem>

namespace vifo {
namespace {
struct Scanner : path::ListScanner {
	[[nodiscard]] auto should_store([[maybe_unused]] fs::path const& path) const -> bool final { return list_files || fs::is_directory(path); }

	bool list_files{};
};

[[nodiscard]] auto format_path(PatternSwapFormatter& formatter, fs::path const& source) -> fs::path {
	auto const source_stem = source.stem().string();
	auto const destination_stem = formatter.format_string(source_stem);
	if (destination_stem.empty()) { return {}; }

	auto ret = util::prefix_parent(source, destination_stem);
	ret += source.extension();
	return ret;
}
} // namespace

auto PatternSwapGenerator::create(Format directory, std::optional<Format> file) -> Result<PatternSwapGenerator> {
	auto ret = PatternSwapGenerator{};
	return ret.create_directory(std::move(directory)).and_then([&] { return ret.create_file(std::move(file)); }).transform([&] { return std::move(ret); });
}

auto PatternSwapGenerator::generate_manifest(fs::path const& directory) -> Result<Manifest> {
	if (!fs::is_directory(directory)) { return detail::to_error(Error::Type::Argument, std::format("not a directory: '{}'", directory.generic_string())); }

	auto scanner = Scanner{};
	scanner.list_files = m_file.has_value();
	auto path_list = scanner.scan_paths(directory);

	if (path_list.paths.empty()) { return detail::to_error(Error::Type::Identify, std::format("no entries found in: '{}'", directory.generic_string())); }

	auto ret = Manifest{.parent = std::move(path_list.scan_path)};
	for (auto& source : path_list.paths) {
		if (source.empty()) { continue; }

		auto destination = [&] {
			if (fs::is_regular_file(source)) { return format_path(*m_file, source); }
			return format_path(m_directory, source);
		}();
		if (destination.empty()) { continue; }

		if (source == ret.parent) { ret.parent = ret.parent.parent_path(); }
		ret.entries.push_back(Manifest::Entry{.source = std::move(source), .destination = std::move(destination)});
	}
	return ret;
}

auto PatternSwapGenerator::create_directory(Format format) -> Result<void> {
	return PatternSwapFormatter::create(std::move(format)).transform([&](PatternSwapFormatter swapper) { m_directory = std::move(swapper); });
}

auto PatternSwapGenerator::create_file(std::optional<Format> format) -> Result<void> {
	if (!format) { return {}; }
	return PatternSwapFormatter::create(std::move(*format)).transform([&](PatternSwapFormatter swapper) { m_file = std::move(swapper); });
}
} // namespace vifo
