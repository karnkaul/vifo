#pragma once
#include "klib/enum_name.hpp"
#include <cstdint>
#include <string>

namespace vifo {
struct Error {
	enum class Type : std::int8_t { Syntax, Format };

	inline static auto const type_name_map = klib::EnumNameMap<Type>{
		{Type::Syntax, "SyntaxError"},
		{Type::Format, "FormatError"},
	};

	Type type{};
	std::string message{};
};
} // namespace vifo
