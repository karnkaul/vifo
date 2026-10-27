#include "command/metadata.hpp"
#include "vifo/media_file.hpp"
#include <print>
#include <string_view>

namespace vifo::cli::command {
auto Metadata::get_parameters() -> std::vector<clap::Parameter> {
	return {
		clap::positional_required(m_text, "TEXT", "input string to parse title from"),
	};
}

auto Metadata::execute([[maybe_unused]] Config const& config) -> ExitCode {
	std::println("input: {}", m_text);

	auto const parsed_filename = MediaFile::Metadata::build(std::string{m_text});
	std::println("{}", parsed_filename.to_table());

	return ExitCode::Success;
}
} // namespace vifo::cli::command
