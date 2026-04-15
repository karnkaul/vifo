#pragma once
#include "klib/lerp_expr/token.hpp"
#include "vifo/result.hpp"
#include <cstddef>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace vifo::expression {
using Token = klib::lerp_expr::Token;

struct Substring {
	auto consume(std::string_view& out_input) const -> bool;

	std::string text{};
};

struct Identifier {
	std::string name{};
	std::size_t length{};
};

struct Atom {
	std::variant<Substring, Identifier> value{};
	Token token{};
};

struct Expression {
	std::vector<Atom> atoms{};
};

struct Term {
	std::string format{};
	Expression expression{};
};

[[nodiscard]] auto parse(std::string_view input) -> Result<Expression>;
} // namespace vifo::expression
