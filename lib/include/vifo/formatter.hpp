#pragma once
#include "klib/base_types.hpp"
#include "vifo/result.hpp"
#include <memory>
#include <optional>
#include <string>
#include <string_view>

namespace vifo {
class IFormatter : public klib::Polymorphic {
  public:
	/// \returns Transformed string on match, otherwise empty string.
	[[nodiscard]] virtual auto format(std::string_view input) -> std::string = 0;
};

struct PatternSwapFormat {
	[[nodiscard]] static auto from_file(std::string_view path) -> std::optional<PatternSwapFormat>;

	std::string input{};
	std::string output{};
};

[[nodiscard]] auto create_pattern_swapper(PatternSwapFormat format) -> Result<std::unique_ptr<IFormatter>>;
} // namespace vifo
