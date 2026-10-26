#pragma once
#include "vifo/environment.hpp"
#include "vifo/expression.hpp"
#include "vifo/formats.hpp"
#include "vifo/result.hpp"
#include <regex>

namespace vifo::interpolator {
class PatternSwap {
  public:
	using Format = PatternSwapFormat;

	[[nodiscard]] static auto create(Format const& format) -> Result<PatternSwap>;

	/// \returns Transformed string on match, otherwise empty string.
	[[nodiscard]] auto interpolate(std::string_view input) -> std::string;

  private:
	[[nodiscard]] auto build_source(std::string_view format, expression::Expression const& expression) -> Result<void>;
	[[nodiscard]] auto build_destination(std::string_view format, expression::Expression expression) -> Result<void>;
	[[nodiscard]] auto extract_values(std::string_view input) -> bool;

	std::regex m_source{};
	expression::Expression m_destination{};

	Environment m_environment{};
};
} // namespace vifo::interpolator
