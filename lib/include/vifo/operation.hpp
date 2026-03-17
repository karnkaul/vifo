#pragma once
#include "klib/enum_name.hpp"
#include <cstdint>

namespace vifo {
enum class Operation : std::int8_t { Move, Copy };
inline auto const operation_name_map = klib::EnumNameMap<Operation>{
	{Operation::Move, "Move"},
	{Operation::Copy, "Copy"},
};
} // namespace vifo
