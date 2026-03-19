#include "vifo/transaction.hpp"
#include "klib/cli/text_table.hpp"
#include "vifo/util/util.hpp"
#include <filesystem>
#include <ranges>

namespace vifo {
auto Transaction::format_table(fs::path const& parent, std::span<Record const> records) -> std::string {
	auto table = klib::TextTable::Builder{}.add_column("#", klib::TextTable::Align::Right).add_column("destination").add_column("source").build();

	auto row = std::vector<std::string>{};
	for (auto const [index, record] : std::views::enumerate(records)) {
		row.reserve(3);
		row.push_back(std::format("{}", index + 1));
		row.push_back(util::to_relative(parent, record.destination).generic_string());
		row.push_back(util::to_relative(parent, record.source).generic_string());
		table.push_row(std::move(row));
	}
	return table.serialize();
}

void Transaction::triage_record(Record record, Outcome const outcome) {
	switch (outcome) {
	case Outcome::Success: success.push_back(std::move(record)); break;
	case Outcome::Failure: failure.push_back(std::move(record)); break;
	default:
	case Outcome::Pass: pass.push_back(std::move(record)); break;
	}
}
} // namespace vifo
