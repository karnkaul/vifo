#pragma once
#include "command/command.hpp"
#include <string>
#include <string_view>

namespace vifo::cli::command {
class Interpolate : public Command {
	static constexpr std::string_view name_v{"interpolate"};
	static constexpr std::string_view help_v{"rename matching files/directories via interpolation"};

	[[nodiscard]] auto get_name() const -> std::string_view final { return name_v; }
	[[nodiscard]] auto get_help() const -> std::string_view final { return help_v; }

	void populate_args() final;

	[[nodiscard]] auto execute() -> ExitCode final;

	std::string_view m_type{};
	int m_max_depth{10};
	std::string_view m_format_json{};
	std::string m_input_format{};
	std::string m_output_format{};
	std::string m_root{"."};
};
} // namespace vifo::cli::command
