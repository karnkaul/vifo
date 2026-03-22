#pragma once
#include "vifo/expression.hpp"
#include "vifo/transaction.hpp"
#include "vifo/types.hpp"
#include <expected>
#include <string_view>

namespace vifo::detail {
[[nodiscard]] auto to_error(Error::Type type, expression::Token token, std::string_view input, std::string_view msg) -> std::unexpected<Error>;
[[nodiscard]] auto to_error(Error::Type type, std::string_view msg) -> std::unexpected<Error>;

[[nodiscard]] inline auto to_record(fs::path source, fs::path destination, Operation const operation) -> Record {
	return Record{
		.source = std::move(source),
		.destination = std::move(destination),
		.operation = operation,
	};
}
} // namespace vifo::detail
