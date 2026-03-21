#pragma once
#include "vifo/expression.hpp"
#include "vifo/formatter/formatter.hpp"
#include "vifo/result.hpp"

namespace vifo {
struct SubtitleFormat {
	std::string primary{"{title}.en.default"};
	std::string secondary{"{title}.en.sub_{number}"};
};

class SubtitleFormatter : public Formatter {
  public:
	static constexpr std::string_view title_identifier_v{"title"};
	static constexpr std::string_view number_identifier_v{"number"};

	using Format = SubtitleFormat;

	[[nodiscard]] static auto create(Format format = {}) -> Result<SubtitleFormatter>;

	[[nodiscard]] auto format_string(std::string_view input) -> std::string final;

	void set_number(int number);
	void set_title(std::string title);

  private:
	using Expression = expression::Expression;

	[[nodiscard]] auto format_number() -> std::string;

	Format m_format{};
	Expression m_primary{};
	Expression m_secondary{};

	int m_number{};
};
} // namespace vifo
