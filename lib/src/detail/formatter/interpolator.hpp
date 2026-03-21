#pragma once
#include "vifo/formatter.hpp"
#include <memory>

namespace vifo::expression {
struct Atom;
struct Expression;
} // namespace vifo::expression

namespace vifo::detail {
struct InterpolateContext;

class Interpolator : public IFormatter {
  public:
	explicit Interpolator();

	[[nodiscard]] auto initialize(InterpolateFormat format) -> Result<void>;

  private:
	using Context = InterpolateContext;
	using Expression = expression::Expression;

	struct Deleter {
		void operator()(Context* ptr) const noexcept;
	};

	[[nodiscard]] auto format(std::string_view const input) -> std::string final {
		if (!extract_values(input)) { return {}; }
		return interpolate();
	}

	[[nodiscard]] auto build_source(Expression expression) -> Result<void>;
	[[nodiscard]] auto build_transform(Expression transform) -> Result<void>;
	[[nodiscard]] auto extract_values(std::string_view input) -> bool;
	[[nodiscard]] auto match_symbol(std::string_view& out_input, expression::Atom const& atom) -> bool;

	[[nodiscard]] auto interpolate() const -> std::string;

	std::unique_ptr<Context, Deleter> m_context{};
};
} // namespace vifo::detail
