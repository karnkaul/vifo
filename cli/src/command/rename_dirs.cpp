#include "command/rename_dirs.hpp"
#include "klib/args/arg.hpp"
#include "klib/assert.hpp"
#include "vifo/formatter.hpp"
#include "vifo/machine_state.hpp"
#include "vifo/manifest.hpp"
#include "vifo/path/scanner.hpp"
#include "vifo/transaction.hpp"
#include "vifo/types.hpp"
#include "vifo/util/prompt.hpp"
#include "vifo/util/util.hpp"
#include <filesystem>
#include <print>

namespace vifo::cli::command {
namespace {
struct Storage {
	std::string input_format{};
	std::string output_format{};
	fs::path root_path{};

	std::unique_ptr<IFormatter> formatter{};
	std::vector<fs::path> path_list{};
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
		auto table = Transaction::format_table(transaction.failure);
		std::println(stderr, "{}", table);
	}
	if (!transaction.pass.empty()) {
		std::println("pass:");
		auto table = Transaction::format_table(transaction.pass);
		std::println(stderr, "{}", table);
	}
	if (!transaction.success.empty()) {
		std::println("success:");
		auto table = Transaction::format_table(transaction.success);
		std::println(stderr, "{}", table);
	}
}

class State : public MachineState {
  public:
	explicit State(Storage storage, std::string_view name) : MachineState(name), m_storage(std::move(storage)) {}

  protected:
	[[nodiscard]] auto handle_error(Error const& error) -> std::unique_ptr<State> {
		std::println(stderr, "{}", error.message);
		m_exit_code = to_exit_code(error.type);
		return {};
	}

	Storage m_storage{};
};

class StateCreateInterpolator : public State {
  public:
	explicit StateCreateInterpolator(Storage state) : State(std::move(state), "CreateInterpolator") {}

  private:
	auto execute() -> std::unique_ptr<MachineState> final;
};

class StateCollectPaths : public State, public path::Scanner {
  public:
	explicit StateCollectPaths(Storage state) : State(std::move(state), "CollectPaths") {}

  private:
	auto execute() -> std::unique_ptr<MachineState> final;

	[[nodiscard]] auto should_store(fs::path const& path) const -> bool final { return fs::is_directory(path); }
};

class StateBuildManifest : public State {
  public:
	explicit StateBuildManifest(Storage state) : State(std::move(state), "BuildManifest") {}

  private:
	auto execute() -> std::unique_ptr<MachineState> final;
};

class StateTransform : public State {
  public:
	explicit StateTransform(Storage state) : State(std::move(state), "BuildManifest") {}

  private:
	auto execute() -> std::unique_ptr<MachineState> final;

	[[nodiscard]] auto confirm_rename() -> bool;
};

auto StateCreateInterpolator::execute() -> std::unique_ptr<MachineState> {
	auto interpolator = create_interpolator(std::move(m_storage.input_format), std::move(m_storage.output_format));
	if (!interpolator) { return handle_error(interpolator.error()); }

	m_storage.formatter = std::move(*interpolator);
	return std::make_unique<StateCollectPaths>(std::move(m_storage));
}

auto StateCollectPaths::execute() -> std::unique_ptr<MachineState> {
	m_storage.path_list = scan_paths(m_storage.root_path);
	if (m_storage.path_list.empty()) {
		std::println("no paths collected");
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

	std::println("{} entries to rename, {} collision(s)", m_storage.manifest.entries.size(), m_storage.manifest.collision_count);

	return std::make_unique<StateTransform>(std::move(m_storage));
}

auto StateTransform::execute() -> std::unique_ptr<MachineState> {
	if (!confirm_rename()) { return {}; }

	m_storage.transaction = util::transform(m_storage.manifest, Operation::Rename, m_storage.overwrite);
	print_transaction(m_storage.transaction);

	if (m_storage.transaction.success.empty()) { return {}; }

	if (!should_continue("undo?")) {
		if (!m_storage.transaction.failure.empty()) { m_exit_code = ExitCode::TransformFailure; }
		return {};
	}

	m_storage.transaction = util::undo(m_storage.transaction.success);
	if (!m_storage.transaction.failure.empty()) { m_exit_code = ExitCode::TransformFailure; }
	print_transaction(m_storage.transaction);

	std::println("transform complete");

	return {};
}

auto StateTransform::confirm_rename() -> bool {
	if (m_storage.manifest.collision_count == 0) { return should_continue("rename?"); }

	std::println("rename:");
	auto const options = std::array{
		prompt::Option{.text = "overwrite existing", .callback = [this] { m_storage.overwrite = true; }},
		prompt::Option{.text = "skip existing", .callback = [this] { m_storage.overwrite = false; }},
	};
	return should_continue(options);
}
} // namespace

void RenameDirs::populate_args() {
	m_args = {
		klib::args::positional_required(m_input_format, "INPUT_FMT", "input dirname format string"),
		klib::args::positional_required(m_output_format, "OUTPUT_FMT", "output dirname format string"),
		klib::args::positional_optional(m_root, "ROOT", "root directory"),
	};
}

auto RenameDirs::execute() -> ExitCode {
	auto const root = fs::path{m_root};
	if (!fs::exists(root)) {
		std::println(stderr, "invalid path: '{}'", m_root);
		return ExitCode::InvalidArgument;
	}

	auto storage = Storage{
		.input_format = std::move(m_input_format),
		.output_format = std::move(m_output_format),
		.root_path = fs::canonical(root),
	};
	return execute_state_machine(std::make_unique<StateCreateInterpolator>(std::move(storage)));
}
} // namespace vifo::cli::command
