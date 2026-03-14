#pragma once
#include "klib/enum_name.hpp"
#include <cstdint>
#include <string>
#include <string_view>

namespace vifo {
struct Error {
	enum class Type : std::int8_t {
		InvalidArgument,
		SyntaxError,
	};

	inline static auto const type_name_map = klib::EnumNameMap<Type>{
		{Type::InvalidArgument, "InvalidArgument"},
		{Type::SyntaxError, "SyntaxError"},
	};

	[[nodiscard]] static auto from_msg(Type type, std::string_view msg) -> Error;

	Type type{};
	std::string message{};
};
} // namespace vifo
