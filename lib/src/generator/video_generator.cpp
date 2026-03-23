#include "vifo/generator/video_generator.hpp"
#include "detail/common.hpp"
#include "vifo/media/file.hpp"
#include "vifo/util/util.hpp"

namespace vifo {
auto VideoGenerator::get_subtitles_dir_for(fs::path const& media_file) const -> fs::path { return util::prefix_parent(media_file, subtitles_dirname); }

auto VideoGenerator::create_builder(IVideoFormatter& video_formatter, fs::path manifest_parent) -> Builder {
	return Builder{*this, video_formatter, std::move(manifest_parent)};
}

VideoGenerator::Builder::Builder(VideoGenerator& generator, IVideoFormatter& formatter, fs::path manifest_parent)
	: m_generator(generator), m_formatter(formatter) {
	manifest.parent = std::move(manifest_parent);
}

void VideoGenerator::Builder::process_video(MediaFile video, std::vector<MediaFile>& out_subtitles) {
	auto video_entry = Manifest::Entry{.source = std::move(video.path), .type = video.type};
	video_entry.destination = m_formatter.format_video(video_entry.source);

	if (video_entry.destination.empty()) {
		manifest.orphans.push_back(std::move(video_entry));
		for (auto& subtitle : out_subtitles) { manifest.orphans.push_back(Manifest::Entry{.source = std::move(subtitle.path), .type = subtitle.type}); }
		return;
	}

	detail::filter_en_subtitles(manifest, out_subtitles);

	auto const subtitle_directory = m_generator.get_subtitles_dir_for(video_entry.destination);
	m_generator.m_subtitle_formatter.set_title(video_entry.destination.stem().string());
	manifest.entries.push_back(std::move(video_entry));

	for (auto& subtitle : out_subtitles) {
		auto subtitle_entry = Manifest::Entry{.source = std::move(subtitle.path), .type = subtitle.type};
		subtitle_entry.destination = m_generator.m_subtitle_formatter.format_path(subtitle_directory, subtitle_entry.source.extension().string());
		manifest.entries.push_back(std::move(subtitle_entry));
	}
}
} // namespace vifo
