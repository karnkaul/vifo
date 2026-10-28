#include "command/print_metadata.hpp"
#include "vifo/metadata.hpp"
#include <print>
#include <string_view>

namespace vifo::cli::command {
auto PrintMetadata::get_parameters() -> std::vector<clap::Parameter> {
	return {
		clap::positional_required(m_text, "TEXT", "input string to parse metadata from"),
	};
}

auto PrintMetadata::execute() -> ExitCode {
	std::println("input: {}", m_text);

	auto const metadata = Metadata::build(m_text);
	std::println("{}", metadata.to_table());

	return ExitCode::Success;
}
} // namespace vifo::cli::command
