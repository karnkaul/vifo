#include "vifo/path/undo.hpp"
#include "vifo/record.hpp"

namespace vifo {
namespace path {
namespace {
[[nodiscard]] auto undo(Record const& record) -> Record {
	if (record.destination.empty()) { return {.outcome = Outcome::Pass}; }

	auto ret = Record{.source = record.destination};
	auto err = std::error_code{};
	switch (record.operation) {
	case Operation::Copy: {
		ret.operation = Operation::Delete;
		if (!fs::remove(record.destination, err)) { ret.outcome = Outcome::Failure; }
		return ret;
	}
	case Operation::Rename: {
		if (record.source.empty() || fs::exists(record.source)) { return {}; }
		ret.operation = Operation::Rename;
		ret.destination = record.source;
		fs::rename(ret.source, ret.destination, err);
		if (err != std::errc{}) { ret.outcome = Outcome::Failure; }
		return ret;
	}
	default: break;
	}

	return {.outcome = Outcome::Pass};
}
} // namespace
} // namespace path

auto path::undo_successful(std::span<Record const> records) -> std::vector<Record> {
	if (records.empty()) { return {}; }

	auto ret = std::vector<Record>{};
	auto const count = records.size();
	ret.reserve(count);
	for (auto const& record : records) { ret.push_back(undo(record)); }

	return ret;
}
} // namespace vifo
