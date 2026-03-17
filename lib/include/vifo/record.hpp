#pragma once
#include "vifo/operation.hpp"
#include <filesystem>

namespace vifo {
namespace fs = std::filesystem;

struct Record {
	fs::path source{};
	fs::path destination{};
	Operation operation{};
};
} // namespace vifo
