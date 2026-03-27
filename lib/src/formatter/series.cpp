#include "vifo/formatter/series.hpp"
#include "detail/common.hpp"
#include "vifo/manifest.hpp"
#include "vifo/types.hpp"
#include <expected>
#include <filesystem>

namespace vifo::formatter {
auto Series::create(omdb::IService const& omdb_service, Format const& format) -> Result<Series> {
	auto ret = Series{omdb_service};
	return interpolator::Title::create(format.directory).transform([&](interpolator::Title formatter) {
		ret.m_title = std::move(formatter);
		return std::move(ret);
	});
}

auto Series::generate_manifest(fs::path const& directory) -> Result<Manifest> {
	auto path = detail::if_directory(directory);
	if (!path) { return std::unexpected{std::move(path.error())}; }

	auto title = get_search_title(*path);
	if (title.empty()) { return detail::to_error(Error::Type::Identify, std::format("failed to identify title for: '{}'", path->generic_string())); }

	auto omdb_series = m_omdb_service->search_series(title);
	if (!omdb_series) { return detail::to_error(Error::Type::Http, omdb_series.error().text); }

	if (omdb_series->payload.title.empty()) {
		return detail::to_error(Error::Type::Identify, std::format("failed to identify title for: '{}'", path->generic_string()));
	}

	m_title.set_title(std::move(omdb_series->payload.title));
	m_title.set_year(omdb_series->payload.year);

	auto ret = Manifest{};

	auto entry = Manifest::Entry{.source = std::move(*path), .type = MediaFileType::Directory};
	entry.destination = m_title.format_directory(entry.source);
	if (entry.destination.empty()) {
		ret.orphans.push_back(std::move(entry));
	} else {
		ret.entries.push_back(std::move(entry));
	}

	return ret;
}
} // namespace vifo::formatter
