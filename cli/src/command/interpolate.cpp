#include "command/interpolate.hpp"
#include "klib/args/arg.hpp"
#include "klib/debug/assert.hpp"
#include "klib/enum/bitops.hpp"
#include "log.hpp"
#include "vifo/formatter.hpp"
#include "vifo/machine_state.hpp"
#include "vifo/manifest.hpp"
#include "vifo/path/list.hpp"
#include "vifo/path/scanner.hpp"
#include "vifo/transaction.hpp"
#include "vifo/types.hpp"
#include "vifo/util/progress.hpp"
#include <algorithm>
#include <filesystem>
#include <optional>
#include <print>

namespace vifo::cli::command {
namespace {
enum class Type : std::uint8_t {
	Dir = 1 << 0,
	File = 1 << 1,
	Any = Dir | File,
};
[[nodiscard]] constexpr auto enable_enum_bitops(Type /*unused*/) { return true; }

auto const type_name_map = klib::EnumNameMap<Type>{
	{Type::Dir, "dir"},
	{Type::File, "file"},
	{Type::Any, "any"},
};

struct Storage {
	int max_depth{};
	PatternSwapFormat format{};
	fs::path root_path{};
	Type type{};

	std::unique_ptr<IFormatter> formatter{};
	path::List path_list{};
	Manifest manifest{};

	bool overwrite{};
	Transaction transaction{};
};

[[nodiscard]] constexpr auto to_exit_code(Error::Type const type) {
	switch (type) {
	case Error::Type::Syntax: return ExitCode::SyntaxError;
	case Error::Type::Format: return ExitCode::FormatError;
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

class StateCreateInterpolator : public State {
  public:
	explicit StateCreateInterpolator(Storage storage) : State(std::move(storage), "CreateInterpolator") {}

  private:
	auto execute() -> std::unique_ptr<MachineState> final;
};

class StateScanPaths : public State, path::ListScanner {
  public:
	explicit StateScanPaths(Storage storage) : State(std::move(storage), "ScanPaths") {}

  private:
	auto execute() -> std::unique_ptr<MachineState> final;

	[[nodiscard]] auto should_iterate(fs::path const& directory) const -> bool final;
	[[nodiscard]] auto should_store(fs::path const& path) const -> bool final;
};

class StateBuildManifest : public State {
  public:
	explicit StateBuildManifest(Storage storage) : State(std::move(storage), "BuildManifest") {}

  private:
	auto execute() -> std::unique_ptr<MachineState> final;
};

class StateTransform : public State, Manifest::Transformer {
  public:
	explicit StateTransform(Storage storage) : State(std::move(storage), "BuildManifest") {}

  private:
	auto execute() -> std::unique_ptr<MachineState> final;
	void on_transformed(Record const& record, Outcome outcome) const final;

	[[nodiscard]] auto confirm_rename() -> bool;

	std::unique_ptr<util::Progress> m_progress{};
};

auto StateCreateInterpolator::execute() -> std::unique_ptr<MachineState> {
	auto pattern_swapper = create_pattern_swapper(std::move(m_storage.format));
	if (!pattern_swapper) { return handle_error(pattern_swapper.error()); }

	m_storage.formatter = std::move(*pattern_swapper);
	return std::make_unique<StateScanPaths>(std::move(m_storage));
}

auto StateScanPaths::should_iterate(fs::path const& directory) const -> bool {
	auto const filename = directory.filename().string();
	return !filename.starts_with('.');
}

auto StateScanPaths::should_store(fs::path const& path) const -> bool {
	auto ret = false;
	if ((m_storage.type & Type::Dir) == Type::Dir) { ret |= fs::is_directory(path); }
	if ((m_storage.type & Type::File) == Type::File) { ret |= fs::is_regular_file(path); }
	return ret;
}

auto StateScanPaths::execute() -> std::unique_ptr<MachineState> {
	m_log.info("filtering by entry type: {}", type_name_map.to_name(m_storage.type));
	max_depth = std::max(0, m_storage.max_depth);
	m_log.debug("max depth: {}", max_depth);
	m_storage.path_list = scan_paths(m_storage.root_path);
	if (m_storage.path_list.paths.empty()) {
		std::println("no paths scanned");
		return {};
	}

	return std::make_unique<StateBuildManifest>(std::move(m_storage));
}

auto StateBuildManifest::execute() -> std::unique_ptr<MachineState> {
	KLIB_ASSERT(m_storage.formatter);
	m_storage.manifest = Manifest::build(*m_storage.formatter, m_storage.path_list);
	if (m_storage.manifest.entries.empty()) {
		std::println("nothing to rename");
		return {};
	}

	std::println("parent: {}", m_storage.manifest.parent.generic_string());
	std::println("{}", m_storage.manifest.format_table());

	if (m_storage.manifest.metrics.duplicates > 0) {
		return set_error(ExitCode::DuplicateDestinations, std::format("{} duplicate destinations! aborting", m_storage.manifest.metrics.duplicates + 1));
	}

	std::println("{} entries to rename, {} existing", m_storage.manifest.entries.size(), m_storage.manifest.metrics.existing);

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
	if (m_storage.manifest.metrics.existing == 0) { return should_continue("rename?"); }

	std::println("rename:");
	auto const options = std::array{
		prompt::Option{.text = "overwrite existing", .callback = [this] { m_storage.overwrite = true; }},
		prompt::Option{.text = "skip existing", .callback = [this] { m_storage.overwrite = false; }},
	};
	return should_continue(options);
}
} // namespace

void Interpolate::populate_args() {
	m_args = {
		klib::args::named_option(m_type, "t,type", "entry type (dir|file|any)"),
		klib::args::named_option(m_max_depth, "d,depth", "max subdirectory depth"),
		klib::args::named_option(m_format_json, "f,format-json", "path to json specifying InterpolateFormat"),
		klib::args::named_option(m_input_format, "i,input", "input dirname format string"),
		klib::args::named_option(m_output_format, "o,output", "output dirname format string"),
		klib::args::positional_optional(m_root, "ROOT", "root directory"),
	};
}

auto Interpolate::execute() -> ExitCode {
	auto const root = fs::path{m_root};
	if (!fs::exists(root)) {
		std::println(stderr, "invalid path: '{}'", m_root);
		return ExitCode::InvalidArgument;
	}

	auto format = PatternSwapFormat{};
	if (!m_format_json.empty()) {
		auto fmt = PatternSwapFormat::from_file(m_format_json);
		if (!fmt) {
			std::println(stderr, "failed to read format json: '{}'", m_format_json);
			return ExitCode::IoError;
		}
		format = std::move(*fmt);
		log.debug("InterpolateFormat extracted from '{}'", m_format_json);
		log.debug("i: {}, o: {}", format.input, format.output);
	} else {
		if (m_input_format.empty() || m_output_format.empty()) {
			std::println(stderr, "either format json or input and output formats are required");
			return ExitCode::InvalidArgument;
		}
		format = PatternSwapFormat{.input = std::move(m_input_format), .output = std::move(m_output_format)};
	}

	auto storage = Storage{
		.max_depth = m_max_depth,
		.format = std::move(format),
		.root_path = fs::canonical(root),
		.type = type_name_map.to_enum(m_type).value_or(Type::Dir),
	};
	return execute_state_machine(std::make_unique<StateCreateInterpolator>(std::move(storage)));
}
} // namespace vifo::cli::command
