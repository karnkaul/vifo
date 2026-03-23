#pragma once
#include "vifo/environment.hpp"
#include "vifo/expression.hpp"
#include "vifo/result.hpp"
#include <string_view>

namespace vifo::interpolator {
struct PatternSwapFormat {
	std::string_view input{};
	std::string_view output{};
};

class PatternSwap {
  public:
	using Format = PatternSwapFormat;

	[[nodiscard]] static auto create(Format const& format) -> Result<PatternSwap>;

	/// \returns Transformed string on match, otherwise empty string.
	[[nodiscard]] auto interpolate(std::string_view input) -> std::string;

  private:
	[[nodiscard]] auto build_source(std::string_view format, expression::Expression expression) -> Result<void>;
	[[nodiscard]] auto build_destination(std::string_view format, expression::Expression expression) -> Result<void>;
	[[nodiscard]] auto extract_values(std::string_view input) -> bool;
	[[nodiscard]] auto match_symbol(std::string_view& out_input, expression::Atom const& atom) -> bool;

	expression::Expression m_source{};
	expression::Expression m_destination{};

	Environment m_environment{};
};
} // namespace vifo::interpolator
