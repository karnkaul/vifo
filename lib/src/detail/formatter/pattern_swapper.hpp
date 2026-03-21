#pragma once
#include "vifo/formatter.hpp"
#include <memory>

namespace vifo::expression {
struct Atom;
struct Expression;
} // namespace vifo::expression

namespace vifo::detail {
struct PatternSwapContext;

class PatternSwapper : public IFormatter {
  public:
	explicit PatternSwapper();

	[[nodiscard]] auto initialize(PatternSwapFormat format) -> Result<void>;

  private:
	using Context = PatternSwapContext;
	using Expression = expression::Expression;

	struct Deleter {
		void operator()(Context* ptr) const noexcept;
	};

	[[nodiscard]] auto format_string(std::string_view input) -> std::string final;

	[[nodiscard]] auto build_source(Expression expression) -> Result<void>;
	[[nodiscard]] auto build_transform(Expression transform) -> Result<void>;
	[[nodiscard]] auto extract_values(std::string_view input) -> bool;
	[[nodiscard]] auto match_symbol(std::string_view& out_input, expression::Atom const& atom) -> bool;

	std::unique_ptr<Context, Deleter> m_context{};
};
} // namespace vifo::detail
