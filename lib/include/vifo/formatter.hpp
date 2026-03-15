#pragma once
#include "klib/base_types.hpp"
#include <string>
#include <string_view>

namespace vifo {
class IFormatter : public klib::Polymorphic {
  public:
	[[nodiscard]] virtual auto format(std::string_view input, std::string& output) -> bool = 0;
};
} // namespace vifo
