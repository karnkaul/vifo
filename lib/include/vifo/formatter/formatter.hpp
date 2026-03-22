#pragma once
#include "klib/base_types.hpp"
#include "vifo/environment.hpp"
#include "vifo/expression.hpp"
#include <memory>
#include <string>
#include <string_view>

namespace vifo {
class Formatter : public klib::Polymorphic {
  public:
	/// \returns Transformed string.
	[[nodiscard]] virtual auto format_string(std::string_view input) -> std::string = 0;

  protected:
	[[nodiscard]] auto interpolate(expression::Expression const& expression) const -> std::string;

	std::unique_ptr<Environment> m_environment{std::make_unique<Environment>()};
};
} // namespace vifo
