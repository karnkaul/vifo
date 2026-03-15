#pragma once
#include "vifo/expression.hpp"
#include "vifo/formatter.hpp"
#include <memory>

namespace vifo {
class IInterpolator : public IFormatter {
  public:
	using Expression = expression::Expression;

	[[nodiscard]] static auto create(Expression input, Expression output) -> Result<std::unique_ptr<IInterpolator>>;
};
} // namespace vifo
