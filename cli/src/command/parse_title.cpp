#include "command/parse_title.hpp"
#include "vifo/util/util.hpp"
#include <print>
#include <string_view>

namespace vifo::cli::command {
auto ParseTitle::get_parameters() -> std::vector<clap::Parameter> {
	return {
		clap::positional_required(m_text, "TEXT", "input string to parse title from"),
	};
}

auto ParseTitle::execute([[maybe_unused]] Config const& config) -> ExitCode {
	auto const parsed_title = util::parse_title(m_text);
	std::println("{}", parsed_title);
	return ExitCode::Success;
}
} // namespace vifo::cli::command
