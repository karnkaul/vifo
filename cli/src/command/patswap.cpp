#include "command/patswap.hpp"
#include "klib/args/arg.hpp"
#include "log.hpp"
#include "state/format.hpp"
#include "vifo/formatter/pattern_swap.hpp"
#include "vifo/json_io.hpp"
#include "vifo/machine_state.hpp"
#include "vifo/types.hpp"
#include "vifo/util/util.hpp"
#include <filesystem>
#include <optional>
#include <print>
#include <string_view>

namespace vifo::cli::command {
namespace {
[[nodiscard]] auto format_from_json(dj::Json& out, std::string_view const path) -> std::optional<formatter::PatternSwap::Format> {
	auto json = dj::Json::from_file(path);
	if (!json) { return {}; }
	out = std::move(*json);
	auto ret = formatter::PatternSwap::Format{};
	from_json(out, ret);
	if (ret.directory.input.empty() || ret.directory.output.empty()) { return {}; }
	if (ret.file) {
		if (ret.file->input.empty() || ret.file->output.empty()) { return {}; }
	}
	return ret;
}
} // namespace

void Patswap::populate_args() {
	m_args = {
		klib::args::named_option(m_max_depth, "d,depth", "max subdirectory depth"),
		klib::args::named_option(m_format_json, "f,format-json", "path to json specifying InterpolateFormat"),
		klib::args::positional_optional(m_root, "ROOT", "root directory"),
	};
}

auto Patswap::execute() -> ExitCode {
	auto root = util::path_if_directory(m_root);
	if (root.empty()) { return ExitCode::InvalidArgument; }

	auto format = format_from_json(m_json, m_format_json);
	if (!format) {
		std::println(stderr, "failed to read format json: '{}'", m_format_json);
		return ExitCode::IoError;
	}
	log.debug("InterpolateFormat extracted from '{}'", m_format_json);
	log.debug("d: {} => {}", format->directory.input, format->directory.output);
	if (format->file) { log.debug("f: {} => {}", format->file->input, format->file->output); }

	auto formatter = formatter::PatternSwap::create(*format);
	if (!formatter) { return handle_error(formatter.error()); }

	return execute_state_machine(std::make_unique<StateFormat>(*formatter, std::move(root)));
}
} // namespace vifo::cli::command
