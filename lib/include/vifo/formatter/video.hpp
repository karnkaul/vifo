#pragma once
#include "vifo/formatter/omdb.hpp"
#include "vifo/interpolator/subtitle.hpp"
#include "vifo/interpolator/video.hpp"
#include "vifo/media_file.hpp"

namespace vifo::formatter {
class Video : public Omdb {
  public:
	using Omdb::Omdb;

	[[nodiscard]] auto get_subtitles_dir_for(fs::path const& media_file) const -> fs::path;

	std::string subtitles_dirname{"subs"};

  protected:
	class Processor;

	[[nodiscard]] auto create_video_processor(interpolator::IVideo& interpolator, fs::path manifest_parent) -> Processor;

	interpolator::Subtitle m_subtitle{};
};

class Video::Processor {
  public:
	explicit Processor(Video& formatter, interpolator::IVideo& interpolator, fs::path manifest_parent);

	void process_video(MediaFile const& video, std::vector<MediaFile>& out_subtitles);

	Manifest manifest{};

  private:
	Video& m_formatter;
	interpolator::IVideo& m_interpolator;
};
} // namespace vifo::formatter
