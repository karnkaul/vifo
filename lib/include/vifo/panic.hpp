#pragma once
#include <stdexcept>

namespace vifo {
struct Panic : std::runtime_error {
	using std::runtime_error::runtime_error;
};
} // namespace vifo
