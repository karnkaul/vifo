#pragma once
#include "command/command.hpp"
#include <string_view>

namespace vifo::cli::command {
class GhostCopy : public Command {
	static constexpr std::string_view name_v{"ghostcopy"};
	static constexpr std::string_view help_v{"mirror a directory tree with empty files"};

	[[nodiscard]] auto get_name() const -> std::string_view final { return name_v; }
	[[nodiscard]] auto get_help() const -> std::string_view final { return help_v; }

	auto get_parameters() -> std::vector<clap::Parameter> final;

	[[nodiscard]] auto execute() -> ExitCode final;

	bool m_directories_only{};
	bool m_overwrite{};
	int m_max_depth{10};
	std::string_view m_source{};
	std::string_view m_destination{"."};
};
} // namespace vifo::cli::command
