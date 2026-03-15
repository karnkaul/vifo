#include "vifo/error.hpp"
#include <format>

namespace vifo {
auto ErrorOld::from_msg(Type const type, std::string_view const msg) -> ErrorOld {
	return ErrorOld{
		.type = type,
		.message = std::format("[{}] {}", type_name_map.to_name(type), msg),
	};
}

auto Error::from_msg(Type const type, std::string_view const msg) -> Error {
	return Error{
		.type = type,
		.message = std::format("[{}] {}", type_name_map.to_name(type), msg),
	};
}
} // namespace vifo
