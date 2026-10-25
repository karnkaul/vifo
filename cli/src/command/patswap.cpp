#include "command/patswap.hpp"
#include "log.hpp"
#include "state/format.hpp"
#include "vifo/formatter/pattern_swap.hpp"
#include "vifo/machine_state.hpp"
#include "vifo/types.hpp"
#include "vifo/util/util.hpp"
#include <filesystem>
#include <optional>
#include <string_view>

namespace vifo::cli::command {
auto Patswap::get_parameters() -> std::vector<clap::Parameter> {
	return {
		clap::named_option(m_max_depth, "d,depth", "max subdirectory depth"),
		clap::positional_optional(m_root, "ROOT", "root directory (default = .)"),
	};
}

auto Patswap::execute(Config const& config) -> ExitCode {
	auto root = util::path_if_directory(m_root);
	if (root.empty()) { return ExitCode::InvalidArgument; }

	auto const& format = config.formats.pattern_swap;
	log.debug("d: {} => {}", format.directory.input, format.directory.output);
	if (format.file) { log.debug("f: {} => {}", format.file->input, format.file->output); }

	auto formatter = formatter::PatternSwap::create(format);
	if (!formatter) { return handle_error(formatter.error()); }

	formatter->max_depth = m_max_depth;
	return execute_state_machine(std::make_unique<StateFormat>(*formatter, std::move(root)));
}
} // namespace vifo::cli::command
