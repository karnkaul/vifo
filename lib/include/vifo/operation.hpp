#pragma once
#include "klib/enum_name.hpp"
#include <cstdint>

namespace vifo {
enum class Operation : std::int8_t { Rename, Copy, Delete };
inline auto const operation_name_map = klib::EnumNameMap<Operation>{
	{Operation::Rename, "Rename"},
	{Operation::Copy, "Copy"},
	{Operation::Delete, "Delete"},
};
} // namespace vifo
