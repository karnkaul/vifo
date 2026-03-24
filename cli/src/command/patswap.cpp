#include "command/patswap.hpp"
#include "djson/json.hpp"
#include "klib/args/arg.hpp"
#include "log.hpp"
#include "state/format.hpp"
#include "vifo/formatter/pattern_swap.hpp"
#include "vifo/machine_state.hpp"
#include "vifo/types.hpp"
#include "vifo/util/util.hpp"
#include <filesystem>
#include <optional>
#include <print>
#include <string_view>

namespace vifo::cli::command {
namespace {
[[nodiscard]] auto format_from_json(dj::Json& out, std::string_view const path) -> std::optional<interpolator::PatternSwapFormat> {
	auto json = dj::Json::from_file(path);
	if (!json) { return {}; }
	out = std::move(*json);
	auto ret = interpolator::PatternSwapFormat{};
	from_json(out["input"], ret.input);
	from_json(out["output"], ret.output);
	if (ret.input.empty() || ret.output.empty()) { return {}; }
	return ret;
}
} // namespace

void Patswap::populate_args() {
	m_args = {
		klib::args::named_flag(m_scan_files, "s,scan-files", "scan both directories and files"),
		klib::args::named_option(m_max_depth, "d,depth", "max subdirectory depth"),
		klib::args::named_option(m_format_json, "f,format-json", "path to json specifying InterpolateFormat"),
		klib::args::named_option(m_input_format, "i,input", "input dirname format string"),
		klib::args::named_option(m_output_format, "o,output", "output dirname format string"),
		klib::args::positional_optional(m_root, "ROOT", "root directory"),
	};
}

auto Patswap::execute() -> ExitCode {
	auto root = util::path_if_exists(m_root);
	if (root.empty()) { return ExitCode::InvalidArgument; }

	auto directory_format = interpolator::PatternSwapFormat{};
	if (!m_format_json.empty()) {
		auto fmt = format_from_json(m_json, m_format_json);
		if (!fmt) {
			std::println(stderr, "failed to read format json: '{}'", m_format_json);
			return ExitCode::IoError;
		}
		directory_format = *fmt;
		log.debug("InterpolateFormat extracted from '{}'", m_format_json);
		log.debug("i: {}, o: {}", directory_format.input, directory_format.output);
	} else {
		if (m_input_format.empty() || m_output_format.empty()) {
			std::println(stderr, "either format json or input and output formats are required");
			return ExitCode::InvalidArgument;
		}
		directory_format = interpolator::PatternSwapFormat{.input = m_input_format, .output = m_output_format};
	}

	auto file_format = std::optional<interpolator::PatternSwapFormat>{};
	if (m_scan_files) { file_format = directory_format; }
	auto formatter = formatter::PatternSwap::create(directory_format, file_format);
	if (!formatter) { return handle_error(formatter.error()); }

	return execute_state_machine(std::make_unique<StateFormat>(*formatter, std::move(root)));
}
} // namespace vifo::cli::command
