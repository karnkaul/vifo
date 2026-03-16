#include "command/rename_dirs.hpp"
#include "klib/args/arg.hpp"
#include "klib/text_table.hpp"
#include "vifo/directory/renamer.hpp"
#include "vifo/exit_code.hpp"
#include <print>
#include <ranges>

namespace vifo::cli::command {
namespace {
[[nodiscard]] constexpr auto to_exit_code(Error::Type const type) {
	switch (type) {
	case Error::Type::Syntax: return ExitCode::SyntaxError;
	case Error::Type::Format: return ExitCode::FormatError;
	default: return ExitCode::Failure;
	}
}
} // namespace

void RenameDirs::populate_args() {
	m_args = {
		klib::args::positional_required(m_input_format, "INPUT_FMT", "input dirname format string"),
		klib::args::positional_required(m_output_format, "OUTPUT_FMT", "output dirname format string"),
		klib::args::positional_optional(m_root, "ROOT", "root directory"),
	};
}

auto RenameDirs::execute() -> ExitCode {
	auto renamer = directory::Renamer::create_interpolator(m_input_format, m_output_format);
	if (!renamer) {
		std::println(stderr, "{}", renamer.error().message);
		return to_exit_code(renamer.error().type);
	}

	auto manifest = renamer->build_manifest(m_root);
	std::println("manifest root: {}", manifest.root.generic_string());
	auto table = klib::TextTable::Builder{}.add_column("#", klib::TextTable::Align::Right).add_column("destination").add_column("source").build();
	auto row = std::vector<std::string>{};
	row.reserve(3);
	for (auto const [index, entry] : std::views::enumerate(manifest.entries)) {
		row.push_back(std::format("{}", index + 1));
		row.push_back(entry.destination.generic_string());
		row.push_back(entry.source.generic_string());
		table.push_row(std::move(row));
	}
	std::println("{}", table.serialize());

	return ExitCode::Success;
}
} // namespace vifo::cli::command
