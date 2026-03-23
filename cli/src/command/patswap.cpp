#include "command/patswap.hpp"
#include "djson/json.hpp"
#include "klib/args/arg.hpp"
#include "klib/cli/prompt.hpp"
#include "log.hpp"
#include "vifo/formatter/pattern_swap.hpp"
#include "vifo/machine_state.hpp"
#include "vifo/manifest.hpp"
#include "vifo/transaction.hpp"
#include "vifo/types.hpp"
#include "vifo/util/progress.hpp"
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

	formatter::PatternSwap formatter{};
	Manifest manifest{};
	Manifest::Metrics metrics{};

	bool overwrite{};
	Transaction transaction{};
};

[[nodiscard]] constexpr auto to_exit_code(Error::Type const type) {
	switch (type) {
	case Error::Type::Argument: return ExitCode::InvalidArgument;
	case Error::Type::Syntax: return ExitCode::SyntaxError;
	case Error::Type::Format: return ExitCode::FormatError;
	case Error::Type::Identify: return ExitCode::IdentifyError;
	case Error::Type::Http: return ExitCode::HttpError;
	default: return ExitCode::Failure;
	}
}

void print_transaction(Transaction const& transaction) {
	if (!transaction.failure.empty()) {
		std::println(stderr, "[!] some transforms failed:");
		auto table = Transaction::format_table(transaction.parent, transaction.failure);
		std::println(stderr, "{}", table);
	}
	if (!transaction.pass.empty()) {
		std::println("pass:");
		auto table = Transaction::format_table(transaction.parent, transaction.pass);
		std::println(stderr, "{}", table);
	}
	if (!transaction.success.empty()) {
		std::println("success:");
		auto table = Transaction::format_table(transaction.parent, transaction.success);
		std::println(stderr, "{}", table);
	}
}

class State : public MachineState {
  public:
	explicit State(Storage storage, std::string_view name) : MachineState(name), m_storage(std::move(storage)) {}

  protected:
	[[nodiscard]] auto handle_error(Error const& error) -> std::unique_ptr<MachineState> { return set_error(to_exit_code(error.type), error.message); }

	Storage m_storage{};
};

class StateCreateFormatter : public State {
  public:
	explicit StateCreateFormatter(Storage storage) : State(std::move(storage), "CreateFormatter") {}

  private:
	auto execute() -> std::unique_ptr<MachineState> final;
};

class StateBuildManifest : public State {
  public:
	explicit StateBuildManifest(Storage storage) : State(std::move(storage), "BuildManifest") {}

  private:
	auto execute() -> std::unique_ptr<MachineState> final;
};

class StateTransform : public State, Manifest::Transformer {
  public:
	explicit StateTransform(Storage storage) : State(std::move(storage), "Transform") {}

  private:
	auto execute() -> std::unique_ptr<MachineState> final;
	void on_transformed(Record const& record, Outcome outcome) const final;

	[[nodiscard]] auto confirm_rename() -> bool;

	std::unique_ptr<util::Progress> m_progress{};
};

auto StateCreateFormatter::execute() -> std::unique_ptr<MachineState> {
	auto file_format = std::optional<interpolator::PatternSwapFormat>{};
	if (m_storage.scan_files) { file_format = m_storage.format; }
	auto formatter = formatter::PatternSwap::create(m_storage.format, file_format);
	if (!formatter) { return handle_error(formatter.error()); }

	m_storage.formatter = std::move(*formatter);
	return std::make_unique<StateBuildManifest>(std::move(m_storage));
}

auto StateBuildManifest::execute() -> std::unique_ptr<MachineState> {
	auto manifest = m_storage.formatter.generate_manifest(m_storage.root_path);
	if (!manifest) {
		// TODO: override title
		return handle_error(manifest.error());
	}

	m_storage.manifest = std::move(*manifest);
	if (m_storage.manifest.entries.empty()) {
		std::println("nothing to rename");
		return {};
	}

	std::println("parent: {}", m_storage.manifest.parent.generic_string());
	std::println("{}", m_storage.manifest.format_entries_tables());

	m_storage.metrics = m_storage.manifest.compute_metrics();
	if (m_storage.metrics.duplicates > 0) {
		return set_error(ExitCode::DuplicateDestinations, std::format("{} duplicate destinations! aborting", m_storage.metrics.duplicates + 1));
	}

	std::println("{} entries to rename, {} existing", m_storage.manifest.entries.size(), m_storage.metrics.existing);

	return std::make_unique<StateTransform>(std::move(m_storage));
}

auto StateTransform::execute() -> std::unique_ptr<MachineState> {
	if (!confirm_rename()) { return {}; }

	m_progress = std::make_unique<util::Progress>(std::int64_t(m_storage.manifest.entries.size()));
	m_storage.transaction = transform_manifest(m_storage.manifest, Operation::Rename, m_storage.overwrite);
	m_progress->finish();
	print_transaction(m_storage.transaction);

	if (m_storage.transaction.success.empty()) { return set_error(ExitCode::TransformFailure); }

	if (!should_continue("rollback?")) {
		if (!m_storage.transaction.failure.empty()) { m_exit_code = ExitCode::TransformFailure; }
		return {};
	}

	m_storage.transaction = m_storage.transaction.rollback();
	if (!m_storage.transaction.failure.empty()) { m_exit_code = ExitCode::TransformFailure; }
	print_transaction(m_storage.transaction);

	std::println("transform complete");

	return {};
}

void StateTransform::on_transformed(Record const& /*record*/, Outcome const /*outcome*/) const { m_progress->increment_completed(); }

auto StateTransform::confirm_rename() -> bool {
	if (m_storage.metrics.existing == 0) { return should_continue("rename?"); }

	std::println("rename:");
	auto const options = std::array{
		prompt::Option{.text = "overwrite existing", .callback = [this] { m_storage.overwrite = true; }},
		prompt::Option{.text = "skip existing", .callback = [this] { m_storage.overwrite = false; }},
	};
	return should_continue(options);
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
	return execute_state_machine(std::make_unique<StateCreateFormatter>(std::move(storage)));
}
} // namespace vifo::cli::command
