#pragma once
#include "klib/base_types.hpp"
#include "klib/cli/prompt.hpp"
#include "klib/log.hpp"
#include "vifo/types.hpp"
#include <memory>
#include <string_view>
#include <variant>

namespace vifo {
namespace prompt = klib::prompt;

class MachineState : public klib::Polymorphic {
  public:
	explicit MachineState(std::string_view const name) : m_log(name) {}

	[[nodiscard]] auto execute_state() -> std::variant<std::unique_ptr<MachineState>, ExitCode>;

  protected:
	virtual auto execute() -> std::unique_ptr<MachineState> = 0;

	[[nodiscard]] auto set_error(ExitCode exit_code, std::string_view message = {}) -> std::unique_ptr<MachineState>;
	[[nodiscard]] auto handle_error(Error const& error) -> std::unique_ptr<MachineState> { return set_error(to_exit_code(error.type), error.message); }

	[[nodiscard]] auto should_continue(prompt::Selection selection) -> bool;
	[[nodiscard]] auto should_continue(std::string_view message, std::string& out_line) -> bool;
	[[nodiscard]] auto should_continue(std::string_view message = "continue?") -> bool;
	[[nodiscard]] auto should_continue(std::span<prompt::Option const> options) -> bool;

	klib::TaggedLogger m_log;

	ExitCode m_exit_code{ExitCode::Success};
};

[[nodiscard]] auto execute_state_machine(std::unique_ptr<MachineState> entrypoint, int max_iterations = 100) -> ExitCode;
} // namespace vifo
