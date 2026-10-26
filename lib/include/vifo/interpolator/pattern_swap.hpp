#pragma once
#include "vifo/environment.hpp"
#include "vifo/expression.hpp"
#include "vifo/formats.hpp"
#include "vifo/result.hpp"
#include <string_view>

namespace vifo::interpolator {
class PatternSwap {
  public:
	using Format = PatternSwapFormat;

	[[nodiscard]] static auto create(Format const& format) -> Result<PatternSwap>;

	/// \returns Transformed string on match, otherwise empty string.
	[[nodiscard]] auto interpolate(std::string_view input) -> std::string;

  private:
	struct Source {
		expression::Expression pre_variable{};
		expression::Identifier variable{};
		expression::Expression post_variable{};
	};

	[[nodiscard]] auto build_source(std::string_view format, expression::Expression expression) -> Result<void>;
	[[nodiscard]] auto build_destination(std::string_view format, expression::Expression expression) -> Result<void>;
	[[nodiscard]] auto extract_values(std::string_view input) -> bool;
	[[nodiscard]] auto match_symbol(std::string_view& out_input, expression::Atom const& atom, bool front) -> bool;

	auto extract_front(std::string_view& out_input) -> bool;
	auto extract_back(std::string_view& out_input) -> bool;
	auto extract_variable(std::string_view& out_input) -> bool;

	Source m_source{};
	expression::Expression m_destination{};

	Environment m_environment{};
};
} // namespace vifo::interpolator
