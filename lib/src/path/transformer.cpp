#include "vifo/path/transformer.hpp"
#include "vifo/types.hpp"
#include <filesystem>
#include <system_error>

namespace vifo::path {
auto Transformer::remove_if_overwrite(fs::path const& destination) const -> Outcome {
	if (fs::exists(destination)) {
		if (!overwrite) { return Outcome::Pass; }
		auto err = std::error_code{};
		if (!fs::remove_all(destination, err)) { return Outcome::Failure; }
	}
	return Outcome::Success;
}

auto Transformer::transform(fs::path const& source, fs::path const& destination, Operation const operation) const -> Outcome {
	auto err = std::error_code{};
	if (operation == Operation::Delete) {
		if (fs::exists(source) && !fs::remove(source, err)) { return Outcome::Failure; }
		return Outcome::Success;
	}

	if (!fs::exists(source)) { return Outcome::Pass; }

	auto const outcome = remove_if_overwrite(destination);
	if (outcome != Outcome::Success) { return outcome; }

	if (operation == Operation::Copy) {
		fs::copy(source, destination, err);
	} else {
		fs::rename(source, destination, err);
	}
	if (err != std::errc{}) { return Outcome::Failure; }

	return Outcome::Success;
}
} // namespace vifo::path
