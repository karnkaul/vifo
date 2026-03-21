#pragma once
#include "vifo/expression.hpp"
#include "vifo/formatter.hpp"

namespace vifo::detail {
class SubtitleFormatter : public ISubtitleFormatter {
  public:
	using Format = SubtitleFormat;

	[[nodiscard]] auto initialize(Format format) -> Result<void>;

  private:
	using Expression = expression::Expression;

	[[nodiscard]] auto format_string(std::string_view input) -> std::string final;
	void set_number(int const number) final { m_number = number; }
	void set_title(std::string title) final { m_title = std::move(title); }

	[[nodiscard]] auto format_number() -> std::string;

	Format m_format{};
	Expression m_primary{};
	Expression m_secondary{};

	std::string m_title{};
	int m_number{};
};
} // namespace vifo::detail
