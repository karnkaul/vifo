#pragma once
#include "vifo/result.hpp"
#include <cstddef>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace vifo::expression {
struct Substring {
	auto consume(std::string_view& out_input) const -> bool;

	std::string text{};
};

struct Identifier {
	std::string name{};
	std::size_t length{};
};

using Atom = std::variant<Substring, Identifier>;

struct Expression {
	std::vector<Atom> atoms{};
};

[[nodiscard]] auto parse(std::string_view input) -> Result<Expression>;
} // namespace vifo::expression
