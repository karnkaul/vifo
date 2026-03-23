#pragma once
#include "vifo/environment.hpp"
#include "vifo/expression.hpp"
#include "vifo/result.hpp"
#include <filesystem>
#include <string_view>

namespace vifo::interpolator {
namespace fs = std::filesystem;

struct SubtitleFormat {
	std::string_view output{"{title}.en.sub_{number}"};
};

class Subtitle {
  public:
	static constexpr std::string_view title_identifier_v{"title"};
	static constexpr std::string_view number_identifier_v{"number"};

	using Format = SubtitleFormat;

	[[nodiscard]] static auto create(Format const& format = {}) -> Result<Subtitle>;

	void set_number(int number);
	void set_title(std::string title);

	/// \returns Next subtitle file stem.
	[[nodiscard]] auto interpolate_stem() -> std::string;
	[[nodiscard]] auto interpolate_path(fs::path const& parent, std::string_view extension) -> fs::path;

  private:
	using Expression = expression::Expression;

	Expression m_output{};

	Environment m_environment{};
	int m_number{};
};
} // namespace vifo::interpolator
