#pragma once
#include "command/command.hpp"
#include <string_view>

namespace vifo::cli::command {
class Metadata : public Command {
	static constexpr std::string_view name_v{"metadata"};
	static constexpr std::string_view help_v{"parse MediaFile Metadata from an input string"};

	[[nodiscard]] auto get_name() const -> std::string_view final { return name_v; }
	[[nodiscard]] auto get_help() const -> std::string_view final { return help_v; }

	auto get_parameters() -> std::vector<clap::Parameter> final;

	[[nodiscard]] auto execute(Config const& config) -> ExitCode final;

	std::string_view m_text{};
};
} // namespace vifo::cli::command
