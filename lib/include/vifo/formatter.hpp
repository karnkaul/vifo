#pragma once
#include "klib/base_types.hpp"
#include "vifo/result.hpp"
#include <memory>
#include <string>
#include <string_view>

namespace vifo {
class IFormatter : public klib::Polymorphic {
  public:
	/// \returns Transformed string on match, otherwise empty string.
	[[nodiscard]] virtual auto format(std::string_view input) -> std::string = 0;
};

[[nodiscard]] auto create_interpolator(std::string_view input_format, std::string_view output_format) -> Result<std::unique_ptr<IFormatter>>;
} // namespace vifo
