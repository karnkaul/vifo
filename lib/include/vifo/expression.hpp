#pragma once
#include "klib/enum_name.hpp"
#include "vifo/result.hpp"
#include <cstddef>
#include <cstdint>
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

using Atom = std::variant<Substring, Identifier>;

struct Expression {
	std::vector<Atom> atoms{};
};

template <typename Type>
struct Pipeline {
	Type input{};
	Type output{};
};

[[nodiscard]] auto parse(std::string_view input) -> Result<Expression>;

[[nodiscard]] auto parse_pipeline(Pipeline<std::string_view> input_pipeline) -> Result<Pipeline<Expression>>;
} // namespace vifo::expression
