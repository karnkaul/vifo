#pragma once
#include "klib/enum_name.hpp"
#include "vifo/operation.hpp"
#include <filesystem>

namespace vifo {
namespace fs = std::filesystem;

enum class Outcome : std::int8_t { Success, Failure, Pass };
inline auto const outcome_name_map = klib::EnumNameMap<Outcome>{
	{Outcome::Success, "Success"},
	{Outcome::Failure, "Failure"},
	{Outcome::Pass, "Pass"},
};

struct Record {
	fs::path source{};
	fs::path destination{};
	Operation operation{};
	Outcome outcome{};
};
} // namespace vifo
