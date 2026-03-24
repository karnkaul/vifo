#include "command/patswap.hpp"
#include "djson/json.hpp"
#include "klib/args/arg.hpp"
#include "log.hpp"
#include "state/transform.hpp"
#include "vifo/formatter/pattern_swap.hpp"
#include "vifo/machine_state.hpp"
#include "vifo/manifest.hpp"
#include "vifo/types.hpp"
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

struct Storage {
	int max_depth{};
	interpolator::PatternSwapFormat format{};
	fs::path root_path{};
	bool scan_files{};
};

class StateBuildManifest : public MachineState {
  public:
	explicit StateBuildManifest(Storage storage) : MachineState("BuildManifest"), m_storage(std::move(storage)) {}

  private:
	auto execute() -> std::unique_ptr<MachineState> final {
		auto file_format = std::optional<interpolator::PatternSwapFormat>{};
		if (m_storage.scan_files) { file_format = m_storage.format; }
		auto formatter = formatter::PatternSwap::create(m_storage.format, file_format);
		if (!formatter) { return handle_error(formatter.error()); }

		auto manifest = formatter->generate_manifest(m_storage.root_path);
		if (!manifest) {
			// TODO: override title
			return handle_error(manifest.error());
		}

		if (manifest->entries.empty()) {
			std::println("nothing to rename");
			return {};
		}

		std::println("parent: {}", manifest->parent.generic_string());
		std::println("{}", manifest->format_entries_tables());

		auto const metrics = manifest->compute_metrics();
		if (metrics.duplicates > 0) {
			return set_error(ExitCode::DuplicateDestinations, std::format("{} duplicate destinations! aborting", metrics.duplicates + 1));
		}

		std::println("{} entries to rename, {} existing", manifest->entries.size(), metrics.existing);

		return std::make_unique<StateTransform>(std::move(*manifest));
	}

	Storage m_storage{};
};
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
	auto const root = fs::path{m_root};
	if (!fs::exists(root)) {
		std::println(stderr, "invalid path: '{}'", m_root);
		return ExitCode::InvalidArgument;
	}

	auto format = interpolator::PatternSwapFormat{};
	if (!m_format_json.empty()) {
		auto fmt = format_from_json(m_json, m_format_json);
		if (!fmt) {
			std::println(stderr, "failed to read format json: '{}'", m_format_json);
			return ExitCode::IoError;
		}
		format = *fmt;
		log.debug("InterpolateFormat extracted from '{}'", m_format_json);
		log.debug("i: {}, o: {}", format.input, format.output);
	} else {
		if (m_input_format.empty() || m_output_format.empty()) {
			std::println(stderr, "either format json or input and output formats are required");
			return ExitCode::InvalidArgument;
		}
		format = interpolator::PatternSwapFormat{.input = m_input_format, .output = m_output_format};
	}

	auto storage = Storage{
		.max_depth = m_max_depth,
		.format = format,
		.root_path = fs::canonical(root),
		.scan_files = m_scan_files,
	};
	return execute_state_machine(std::make_unique<StateBuildManifest>(std::move(storage)));
}
} // namespace vifo::cli::command
