#pragma once
#include "klib/base_types.hpp"
#include "vifo/result.hpp"
#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <string_view>

namespace vifo {
namespace fs = std::filesystem;

class IFormatter : public klib::Polymorphic {
  public:
	/// \returns Transformed string on match, otherwise empty string.
	[[nodiscard]] virtual auto format_string(std::string_view input) -> std::string = 0;
	[[nodiscard]] auto format_path(fs::path const& path) -> fs::path;
};

class ISubtitleFormatter : public IFormatter {
  public:
	static constexpr std::string_view title_identifier_v{"title"};
	static constexpr std::string_view number_identifier_v{"number"};

	virtual void set_title(std::string title) = 0;
	virtual void set_number(int number) = 0;
	void reset_number() { set_number(0); }
};

struct PatternSwapFormat {
	[[nodiscard]] static auto from_file(std::string_view path) -> std::optional<PatternSwapFormat>;

	std::string input{};
	std::string output{};
};

struct SubtitleFormat {
	std::string primary{"{title}.en.default"};
	std::string secondary{"{title}.en.sub_{number}"};
};

[[nodiscard]] auto create_pattern_swapper(PatternSwapFormat format) -> Result<std::unique_ptr<IFormatter>>;
[[nodiscard]] auto create_subtitle_formatter(SubtitleFormat format = {}) -> Result<std::unique_ptr<ISubtitleFormatter>>;
} // namespace vifo
