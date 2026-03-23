#pragma once
#include "command/command.hpp"
#include "djson/json.hpp"
#include <string_view>

namespace vifo::cli::command {
class Patswap : public Command {
	static constexpr std::string_view name_v{"patswap"};
	static constexpr std::string_view help_v{"rename matching files/directories via pattern swapping"};

	[[nodiscard]] auto get_name() const -> std::string_view final { return name_v; }
	[[nodiscard]] auto get_help() const -> std::string_view final { return help_v; }

	void populate_args() final;

	[[nodiscard]] auto execute() -> ExitCode final;

	std::string_view m_type{};
	int m_max_depth{10};
	std::string_view m_format_json{};
	dj::Json m_json{};
	std::string_view m_input_format{};
	std::string_view m_output_format{};
	std::string_view m_root{"."};
};
} // namespace vifo::cli::command
