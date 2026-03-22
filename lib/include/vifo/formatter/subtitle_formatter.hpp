#pragma once
#include "vifo/environment.hpp"
#include "vifo/expression.hpp"
#include "vifo/result.hpp"

namespace vifo {
struct SubtitleFormat {
	std::string_view primary{"{title}.en.default"};
	std::string_view secondary{"{title}.en.sub_{number}"};
};

class SubtitleFormatter {
  public:
	static constexpr std::string_view title_identifier_v{"title"};
	static constexpr std::string_view number_identifier_v{"number"};

	using Format = SubtitleFormat;

	[[nodiscard]] static auto create(Format const& format = {}) -> Result<SubtitleFormatter>;

	void set_number(int number);
	void set_title(std::string title);

	/// \returns Next subtitle file stem.
	[[nodiscard]] auto format_stem() -> std::string;

  private:
	using Expression = expression::Expression;

	Expression m_primary{};
	Expression m_secondary{};

	Environment m_environment{};
	int m_number{};
};
} // namespace vifo
