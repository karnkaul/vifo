#pragma once
#include "vifo/environment.hpp"
#include "vifo/expression.hpp"
#include "vifo/formats.hpp"
#include "vifo/result.hpp"
#include <filesystem>
#include <string_view>

namespace vifo::interpolator {
namespace fs = std::filesystem;

class Subtitle {
  public:
	using Format = SubtitleFormat;

	static constexpr std::string_view title_identifier_v{"title"};
	static constexpr std::string_view number_identifier_v{"number"};

	[[nodiscard]] static auto create(Format format = {}) -> Result<Subtitle>;

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
