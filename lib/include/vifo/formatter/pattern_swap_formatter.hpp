#pragma once
#include "vifo/environment.hpp"
#include "vifo/expression.hpp"
#include "vifo/result.hpp"

namespace vifo {
struct PatternSwapFormat {
	[[nodiscard]] static auto from_file(std::string_view path) -> std::optional<PatternSwapFormat>;

	std::string input{};
	std::string output{};
};

class PatternSwapFormatter {
  public:
	using Format = PatternSwapFormat;

	[[nodiscard]] static auto create(Format format) -> Result<PatternSwapFormatter>;

	/// \returns Transformed string on match, otherwise empty string.
	[[nodiscard]] auto format_string(std::string_view input) -> std::string;

  private:
	[[nodiscard]] auto build_source(expression::Expression expression) -> Result<void>;
	[[nodiscard]] auto build_destination(expression::Expression destination) -> Result<void>;
	[[nodiscard]] auto extract_values(std::string_view input) -> bool;
	[[nodiscard]] auto match_symbol(std::string_view& out_input, expression::Atom const& atom) -> bool;

	expression::Term m_source{};
	expression::Term m_destination{};

	Environment m_environment{};
};
} // namespace vifo
