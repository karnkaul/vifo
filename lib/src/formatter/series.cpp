#include "vifo/formatter/series.hpp"
#include "detail/common.hpp"
#include "vifo/formatter/season.hpp"
#include "vifo/manifest.hpp"
#include "vifo/types.hpp"
#include "vifo/util/util.hpp"
#include <expected>
#include <filesystem>

namespace vifo::formatter {
auto Series::create(omdb::IService const& omdb_service, Format const& format) -> Result<Series> {
	auto ret = Series{omdb_service};
	ret.m_season_format = format.season;
	return interpolator::Title::create(format.directory).transform([&](interpolator::Title formatter) {
		ret.m_title = std::move(formatter);
		return std::move(ret);
	});
}

auto Series::do_generate_manifest(fs::path const& directory) -> Result<Manifest> {
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

	auto ret = Manifest{.parent = path->parent_path()};

	auto series_directory = Manifest::Entry{.source = util::to_relative(ret.parent, *path), .type = MediaFileType::Directory};
	series_directory.destination = m_title.format_directory(series_directory.source);
	if (series_directory.destination.empty()) {
		ret.orphans.push_back(std::move(series_directory));
		return ret;
	}

	add_seasons(ret, series_directory);

	if (ret.entries.empty() && series_directory.source != series_directory.destination) { ret.entries.push_back(std::move(series_directory)); }

	return ret;
}

void Series::add_seasons(Manifest& out, Manifest::Entry const& series_directory) const {
	auto season = Season::create(*m_omdb_service, m_season_format);
	if (!season) { return; }

	auto const source = out.parent / series_directory.source;
	for (auto const& it : fs::directory_iterator{source}) {
		if (!it.is_directory()) { continue; }

		auto manifest = season->generate_manifest(it.path());
		if (!manifest) { continue; }

		out.merge_child(*manifest, series_directory.destination);
	}
}
} // namespace vifo::formatter
