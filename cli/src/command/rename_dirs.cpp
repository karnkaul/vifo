#include "command/rename_dirs.hpp"
#include "klib/args/arg.hpp"
#include "klib/assert.hpp"
#include "klib/text_table.hpp"
#include "vifo/exit_code.hpp"
#include "vifo/formatter.hpp"
#include "vifo/machine_state.hpp"
#include "vifo/manifest.hpp"
#include <print>
#include <ranges>

namespace vifo::cli::command {
namespace {
struct Storage {
	std::string input_format{};
	std::string output_format{};
	std::string root_directory{"."};
	std::unique_ptr<IFormatter> formatter{};
	Manifest manifest{};
};

[[nodiscard]] constexpr auto to_exit_code(Error::Type const type) {
	switch (type) {
	case Error::Type::Syntax: return ExitCode::SyntaxError;
	case Error::Type::Format: return ExitCode::FormatError;
	default: return ExitCode::Failure;
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

class StateBuildManifest : public State {
  public:
	explicit StateBuildManifest(Storage state) : State(std::move(state), "BuildManifest") {}

  private:
	auto execute() -> std::unique_ptr<MachineState> final;

	[[nodiscard]] auto confirm_overwrite() -> bool;
};

auto StateCreateInterpolator::execute() -> std::unique_ptr<MachineState> {
	auto interpolator = create_interpolator(std::move(m_storage.input_format), std::move(m_storage.output_format));
	if (!interpolator) { return handle_error(interpolator.error()); }

	m_storage.formatter = std::move(*interpolator);
	return std::make_unique<StateBuildManifest>(std::move(m_storage));
}

auto StateBuildManifest::execute() -> std::unique_ptr<MachineState> {
	KLIB_ASSERT(m_storage.formatter);
	m_storage.manifest = Manifest::build(*m_storage.formatter, m_storage.root_directory);
	std::println("manifest root: {}", m_storage.manifest.root.generic_string());
	auto table = klib::TextTable::Builder{}.add_column("#", klib::TextTable::Align::Right).add_column("destination").add_column("source").build();
	auto row = std::vector<std::string>{};
	for (auto const [index, entry] : std::views::enumerate(m_storage.manifest.entries)) {
		row.reserve(3);
		row.push_back(std::format("{}", index + 1));
		row.push_back(entry.destination.generic_string());
		row.push_back(entry.source.generic_string());
		table.push_row(std::move(row));
	}
	std::println("{}", table.serialize());

	std::println("{} entries to rename, {} collision(s)", m_storage.manifest.entries.size(), m_storage.manifest.collision_count);
	if (!should_continue()) { return {}; }

	m_log.debug("TODO");
	return {};
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
	auto storage = Storage{
		.input_format = std::move(m_input_format),
		.output_format = std::move(m_output_format),
		.root_directory = std::move(m_root),
	};
	return execute_state_machine(std::make_unique<StateCreateInterpolator>(std::move(storage)));
}
} // namespace vifo::cli::command
