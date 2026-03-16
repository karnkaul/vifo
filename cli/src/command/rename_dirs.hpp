#pragma once
#include "command/command.hpp"
#include <string_view>

namespace vifo::cli::command {
class RenameDirs : public Command {
	static constexpr std::string_view name_v{"rename-dirs"};
	static constexpr std::string_view help_v{"rename matching subdirectories via interpolation"};

	[[nodiscard]] auto get_name() const -> std::string_view final { return name_v; }
	[[nodiscard]] auto get_help() const -> std::string_view final { return help_v; }

	void populate_args() final;

	[[nodiscard]] auto execute() -> ExitCode final;

	std::string m_input_format{};
	std::string m_output_format{};
	std::string_view m_root{"."};
};
} // namespace vifo::cli::command
