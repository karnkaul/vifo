#include "vifo/formatter/video.hpp"
#include "detail/common.hpp"
#include "vifo/media_file.hpp"
#include "vifo/util/util.hpp"

namespace vifo::formatter {
auto Video::get_subtitles_dir_for(fs::path const& media_file) const -> fs::path { return util::prefix_parent(media_file, subtitles_dirname); }

auto Video::create_builder(interpolator::IVideo& interpolator, fs::path manifest_parent) -> Builder {
	return Builder{*this, interpolator, std::move(manifest_parent)};
}

Video::Builder::Builder(Video& formatter, interpolator::IVideo& interpolator, fs::path manifest_parent) : m_formatter(formatter), m_interpolator(interpolator) {
	manifest.parent = std::move(manifest_parent);
}

void Video::Builder::process_video(MediaFile video, std::vector<MediaFile>& out_subtitles) {
	auto video_entry = Manifest::Entry{.source = std::move(video.path), .type = video.type};
	video_entry.destination = m_interpolator.interpolate_video(video_entry.source);

	if (video_entry.destination.empty()) {
		manifest.orphans.push_back(std::move(video_entry));
		for (auto& subtitle : out_subtitles) { manifest.orphans.push_back(Manifest::Entry{.source = std::move(subtitle.path), .type = subtitle.type}); }
		return;
	}

	detail::filter_en_subtitles(manifest, out_subtitles);

	auto const subtitle_directory = m_formatter.get_subtitles_dir_for(video_entry.destination);
	m_formatter.m_subtitle.set_title(video_entry.destination.stem().string());
	manifest.entries.push_back(std::move(video_entry));

	for (auto& subtitle : out_subtitles) {
		auto subtitle_entry = Manifest::Entry{.source = std::move(subtitle.path), .type = subtitle.type};
		subtitle_entry.destination = m_formatter.m_subtitle.interpolate_path(subtitle_directory, subtitle_entry.source.extension().string());
		manifest.entries.push_back(std::move(subtitle_entry));
	}
}
} // namespace vifo::formatter
