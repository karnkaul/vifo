#include "command/omdb.hpp"
#include "klib/args/arg.hpp"
#include "state/format.hpp"
#include "vifo/formatter/movie.hpp"
#include "vifo/types.hpp"
#include "vifo/util/util.hpp"

namespace vifo::cli::command {
void Movie::populate_args() {
	m_args = {
		klib::args::positional_required(m_directory, "DIRECTORY", "path to movie directory"),
	};
}

auto Movie::execute() -> ExitCode {
	auto directory = util::path_if_directory(m_directory);
	if (directory.empty()) { return ExitCode::InvalidArgument; }

	auto formatter = formatter::Movie::create(*m_omdb_service);
	if (!formatter) { return handle_error(formatter.error()); }

	return execute_state_machine(std::make_unique<StateFormat>(*formatter, std::move(directory)));
}
} // namespace vifo::cli::command
