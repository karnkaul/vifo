#pragma once
#include "vifo/result.hpp"
#include <string>
#include <string_view>
#include <vector>

namespace vifo {
struct Binding {
	[[nodiscard]] static auto extract(std::string_view format, std::string_view input) -> Result<std::vector<Binding>>;

	std::string key{};
	std::string value{};
};
} // namespace vifo
