#include "vifo/machine_state.hpp"
#include "vifo/types.hpp"
#include "vifo/util/prompt.hpp"
#include <print>

namespace vifo {
using prompt::Option;
using prompt::Selection;

auto MachineState::execute_state() -> std::variant<std::unique_ptr<MachineState>, ExitCode> {
	m_log.debug("executing");
	auto next_stage = execute();
	if (!next_stage) { return m_exit_code; }
	return next_stage;
}

auto MachineState::set_error(ExitCode const exit_code, std::string_view const message) -> std::unique_ptr<MachineState> {
	m_exit_code = exit_code;
	if (!message.empty()) { std::println(stderr, "error: {}", message); }
	return {};
}

auto MachineState::should_continue(Selection const selection) -> bool {
	switch (selection) {
	case Selection::Exit: return false;
	case Selection::Invalid: m_exit_code = ExitCode::InvalidArgument; return false;
	case Selection::Confirm: break;
	}
	return true;
}

auto MachineState::should_continue(std::string_view const message) -> bool {
	auto const selection = util::prompt_confirm(message);
	return should_continue(selection);
}

auto MachineState::should_continue(std::span<Option const> options) -> bool {
	auto selection = util::prompt_options(options, true);
	return should_continue(selection);
}

auto MachineState::should_continue(std::string_view const message, std::string& out_line) -> bool {
	auto const selection = util::prompt_line(message, [&](std::string ret) {
		if (ret.empty()) { return false; }
		out_line = std::move(ret);
		return true;
	});
	return should_continue(selection);
}
} // namespace vifo

auto vifo::execute_state_machine(std::unique_ptr<MachineState> entrypoint, int const max_iterations) -> ExitCode {
	auto iteration = 1;
	for (auto stage = std::move(entrypoint); stage; ++iteration) {
		if (iteration > max_iterations) { return ExitCode::ForcedHalt; }

		auto result = stage->execute_state();
		if (auto* next_stage = std::get_if<std::unique_ptr<MachineState>>(&result)) {
			stage = std::move(*next_stage);
			continue;
		}

		return std::get<ExitCode>(result);
	}

	return ExitCode::Success;
}
