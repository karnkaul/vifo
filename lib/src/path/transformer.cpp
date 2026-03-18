#include "vifo/path/transformer.hpp"
#include "vifo/operation.hpp"
#include "vifo/record.hpp"
#include <filesystem>
#include <system_error>

namespace vifo::path {
namespace {
auto to_record(fs::path source, fs::path destination, Operation const operation) -> Record {
	return Record{
		.source = std::move(source),
		.destination = std::move(destination),
		.operation = operation,
	};
}
} // namespace

auto Transformer::remove_if_overwrite(fs::path const& destination) const -> Outcome {
	if (fs::exists(destination)) {
		if (!overwrite) { return Outcome::Pass; }
		auto err = std::error_code{};
		if (!fs::remove(destination, err)) { return Outcome::Failure; }
	}
	return Outcome::Success;
}

auto Transformer::transform(fs::path const& source, fs::path const& destination, Operation const operation) const -> Record {
	auto ret = to_record(source, destination, operation);

	auto err = std::error_code{};
	if (operation == Operation::Delete) {
		ret.destination.clear();
		if (fs::exists(source) && !fs::remove(source, err)) { ret.outcome = Outcome::Failure; }
		return ret;
	}

	if (!fs::exists(source)) {
		ret.outcome = Outcome::Pass;
		return ret;
	}

	ret.outcome = remove_if_overwrite(destination);
	if (ret.outcome != Outcome::Success) { return ret; }

	if (operation == Operation::Copy) {
		fs::copy(source, destination, err);
	} else {
		fs::rename(source, destination, err);
	}
	if (err != std::errc{}) { ret.outcome = Outcome::Failure; }

	return ret;
}
} // namespace vifo::path
