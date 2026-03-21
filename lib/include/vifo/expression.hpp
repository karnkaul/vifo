#pragma once
#include "klib/enum/name.hpp"
#include "vifo/result.hpp"
#include <cstddef>
#include <cstdint>
#include <functional>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace vifo::expression {
struct Token {
	enum class Type : std::int8_t { Substring, BraceLeft, BraceRight };
	inline static auto const type_name_map = klib::EnumNameMap<Type>{
		{Type::Substring, "Substring"},
		{Type::BraceLeft, "BraceLeft"},
		{Type::BraceRight, "BraceRight"},
	};

	[[nodiscard]] auto get_lexeme(std::string_view text) const -> std::string_view;

	Type type{};
	std::size_t start_index{};
	std::size_t length{};
};

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
	using GetValue = std::move_only_function<std::string_view(Identifier const&)>;

	[[nodiscard]] auto interpolate(GetValue get_value) const -> std::string;

	std::vector<Atom> atoms{};
};

struct Term {
	std::string format{};
	Expression expression{};
};

[[nodiscard]] auto parse(std::string_view input) -> Result<Expression>;
} // namespace vifo::expression
