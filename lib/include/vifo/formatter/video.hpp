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
	class Builder;

	[[nodiscard]] auto create_builder(interpolator::IVideo& interpolator, fs::path manifest_parent) -> Builder;

	interpolator::Subtitle m_subtitle{};
};

class Video::Builder {
  public:
	explicit Builder(Video& formatter, interpolator::IVideo& interpolator, fs::path manifest_parent);

	void process_video(MediaFile video, std::vector<MediaFile>& out_subtitles);

	Manifest manifest{};

  private:
	Video& m_formatter;
	interpolator::IVideo& m_interpolator;
};
} // namespace vifo::formatter
