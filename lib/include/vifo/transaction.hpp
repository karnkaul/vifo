#pragma once
#include "vifo/types.hpp"
#include <filesystem>
#include <span>
#include <vector>

namespace vifo {
namespace fs = std::filesystem;

struct Record {
	fs::path source{};
	fs::path destination{};
	Operation operation{};
};

struct Transaction {
	[[nodiscard]] static auto format_table(std::span<Record const> records) -> std::string;

	void triage_record(Record record, Outcome outcome);

	std::vector<Record> success{};
	std::vector<Record> failure{};
	std::vector<Record> pass{};
};
} // namespace vifo
