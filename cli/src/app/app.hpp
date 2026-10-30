#pragma once
#include "clap/result.hpp"
#include "command/command.hpp"
#include "config/config_loader.hpp"
#include <memory>

namespace vifo::cli {
class App {
  public:
	[[nodiscard]] auto run(int argc, char const* const* argv) -> int;

  private:
	[[nodiscard]] auto parse_args(int argc, char const* const* argv) -> clap::Result;

	template <std::derived_from<Command> T, typename... Args>
		requires(std::constructible_from<T, Args...>)
	void add_command(Args&&... args) {
		m_commands.push_back(std::make_unique<T>(std::forward<Args>(args)...));
	}

	void set_omdb_token();

	ConfigLoader m_config_loader{};
	std::string_view m_config_path{};
	bool m_config_loaded{};

	std::vector<std::unique_ptr<Command>> m_commands{};
};
} // namespace vifo::cli
