#include "vifo/path/undo.hpp"
#include "vifo/transaction.hpp"
#include <utility>

namespace vifo {
namespace path {
namespace {
[[nodiscard]] auto undo(Record const& record) -> std::pair<Record, Outcome> {
	if (record.destination.empty()) { return {{}, Outcome::Pass}; }

	auto ret = Record{.source = record.destination};
	auto outcome = Outcome::Success;

	auto err = std::error_code{};
	switch (record.operation) {
	case Operation::Copy: {
		ret.operation = Operation::Delete;
		if (!fs::remove(record.destination, err)) { outcome = Outcome::Failure; }
		return {std::move(ret), outcome};
	}
	case Operation::Rename: {
		if (record.source.empty() || fs::exists(record.source)) { return {}; }
		ret.operation = Operation::Rename;
		ret.destination = record.source;
		fs::rename(ret.source, ret.destination, err);
		if (err != std::errc{}) { outcome = Outcome::Failure; }
		return {std::move(ret), outcome};
	}
	default: break;
	}

	return {{}, Outcome::Pass};
}
} // namespace
} // namespace path

auto path::undo_successful(std::span<Record const> records) -> Transaction {
	if (records.empty()) { return {}; }

	auto ret = Transaction{};
	for (auto const& record : records) {
		auto [record_out, outcome] = undo(record);
		ret.triage_record(record_out, outcome);
	}

	return ret;
}
} // namespace vifo
