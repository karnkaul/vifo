#pragma once
#include "vifo/formatter/subtitle_formatter.hpp"
#include "vifo/formatter/video_formatter.hpp"
#include "vifo/generator/omdb_generator.hpp"
#include "vifo/media_file.hpp"

namespace vifo {
class VideoGenerator : public OmdbGenerator {
  public:
	using OmdbGenerator::OmdbGenerator;

	[[nodiscard]] auto get_subtitles_dir_for(fs::path const& media_file) const -> fs::path;

	std::string subtitles_dirname{"subs"};

  protected:
	class Builder;

	[[nodiscard]] auto create_builder(IVideoFormatter& video_formatter, fs::path manifest_parent) -> Builder;

	SubtitleFormatter m_subtitle_formatter{};
};

class VideoGenerator::Builder {
  public:
	explicit Builder(VideoGenerator& generator, IVideoFormatter& formatter, fs::path manifest_parent);

	void process_video(MediaFile video, std::vector<MediaFile>& out_subtitles);

	Manifest manifest{};

  private:
	VideoGenerator& m_generator;
	IVideoFormatter& m_formatter;
};
} // namespace vifo
