#include "vifo/generator/series_generator.hpp"
#include "detail/common.hpp"
#include "vifo/formatter/title_formatter.hpp"
#include "vifo/manifest.hpp"
#include "vifo/types.hpp"
#include "vifo/util/util.hpp"
#include <expected>
#include <filesystem>

namespace vifo {
auto SeriesGenerator::create(omdb::IService const& omdb_service, Format const& format) -> Result<SeriesGenerator> {
	auto ret = SeriesGenerator{omdb_service};
	return TitleFormatter::create(format.directory).transform([&](TitleFormatter formatter) {
		ret.m_formatter = std::move(formatter);
		return std::move(ret);
	});
}

auto SeriesGenerator::generate_manifest(fs::path const& directory) -> Result<Manifest> {
	auto path = if_directory(directory);
	if (!path) { return std::unexpected{std::move(path.error())}; }

	auto title = util::identify_title(*path);
	if (title.empty()) { return detail::to_error(Error::Type::Identify, std::format("failed to identify title for: '{}'", path->generic_string())); }

	auto omdb_series = m_omdb_service->search_series(title);
	if (!omdb_series) { return detail::to_error(Error::Type::Http, omdb_series.error().text); }

	if (omdb_series->payload.title.empty()) {
		return detail::to_error(Error::Type::Identify, std::format("failed to identify title for: '{}'", path->generic_string()));
	}

	m_formatter.set_title(std::move(omdb_series->payload.title));
	m_formatter.set_year(omdb_series->payload.year);

	auto ret = Manifest{};

	auto entry = Manifest::Entry{.source = std::move(*path), .type = MediaFileType::Directory};
	entry.destination = m_formatter.format_directory(entry.source);
	if (entry.destination.empty()) {
		ret.orphans.push_back(std::move(entry));
	} else {
		ret.entries.push_back(std::move(entry));
	}

	return ret;
}
} // namespace vifo
