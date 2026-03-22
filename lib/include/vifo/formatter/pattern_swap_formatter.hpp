#pragma once
#include "vifo/expression.hpp"
#include "vifo/formatter/formatter.hpp"
#include "vifo/result.hpp"
#include <memory>

namespace vifo {
struct PatternSwapContext;

struct PatternSwapFormat {
	[[nodiscard]] static auto from_file(std::string_view path) -> std::optional<PatternSwapFormat>;

	std::string input{};
	std::string output{};
};

class PatternSwapFormatter : public Formatter {
  public:
	using Format = PatternSwapFormat;

	[[nodiscard]] static auto create(Format format) -> Result<PatternSwapFormatter>;

	/// \returns Transformed string on match, otherwise empty string.
	[[nodiscard]] auto format_string(std::string_view input) -> std::string override;

  private:
	using Context = PatternSwapContext;
	using Expression = expression::Expression;

	struct Deleter {
		void operator()(Context* ptr) const noexcept;
	};

	[[nodiscard]] auto build_source(Expression expression) -> Result<void>;
	[[nodiscard]] auto build_destination(Expression destination) -> Result<void>;
	[[nodiscard]] auto extract_values(std::string_view input) -> bool;
	[[nodiscard]] auto match_symbol(std::string_view& out_input, expression::Atom const& atom) -> bool;

	std::unique_ptr<Context, Deleter> m_context{};
};
} // namespace vifo
