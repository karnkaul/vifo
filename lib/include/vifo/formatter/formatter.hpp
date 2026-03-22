#pragma once
#include "klib/base_types.hpp"
#include "vifo/environment.hpp"
#include <string>
#include <string_view>

namespace vifo {
class Formatter : public klib::Polymorphic {
  public:
	/// \returns Transformed string.
	[[nodiscard]] virtual auto format_string(std::string_view input) -> std::string = 0;

  protected:
	Environment m_environment{};
};
} // namespace vifo
