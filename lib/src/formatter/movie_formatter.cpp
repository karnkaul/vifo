#include "vifo/formatter/movie_formatter.hpp"

namespace vifo {
auto MovieFormatter::create(MovieFormat const& format) -> Result<MovieFormatter> {
	auto ret = MovieFormatter{};
	return expression::parse(format.video)
		.and_then([&](expression::Expression video) {
			ret.m_video = std::move(video);
			return expression::parse(format.directory);
		})
		.transform([&](expression::Expression directory) {
			ret.m_directory = std::move(directory);
			return std::move(ret);
		});
}

void MovieFormatter::set_movie(omdb::Movie movie) {
	m_environment.set_symbol(title_identifier_v, std::move(movie.title));
	m_environment.set_symbol(year_identifier_v, std::format("{:04}", movie.year));
}

auto MovieFormatter::format_path(fs::path const& video) const -> fs::path {
	auto ret = fs::path{m_environment.interpolate(m_video)};
	ret += video.extension();
	if (!video.has_parent_path()) { return ret; }

	auto parent = video.parent_path();
	parent.replace_filename(m_environment.interpolate(m_directory));
	return parent / ret;
}
} // namespace vifo
