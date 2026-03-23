#include "vifo/formatter/title_formatter.hpp"
#include "vifo/util/util.hpp"
#include <filesystem>
#include <format>

namespace vifo {
auto TitleFormatter::create(Format const& format) -> Result<TitleFormatter> {
	auto ret = TitleFormatter{};
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

void TitleFormatter::set_title(std::string title) { m_environment.set_symbol(title_identifier_v, std::move(title)); }
void TitleFormatter::set_year(int const year) { m_environment.set_symbol(year_identifier_v, std::format("{:02}", year)); }

auto TitleFormatter::format_dirname() const -> std::string { return m_environment.interpolate(m_directory); }

auto TitleFormatter::format_directory(fs::path const& directory) const -> fs::path {
	if (!fs::is_directory(directory)) { return {}; }
	return util::prefix_parent(directory, format_dirname());
}

auto TitleFormatter::format_video(fs::path const& video) -> fs::path {
	auto ret = fs::path{m_environment.interpolate(m_video)};
	ret += video.extension();
	if (!video.has_parent_path()) { return ret; }

	return format_directory(video.parent_path()) / ret;
}
} // namespace vifo
