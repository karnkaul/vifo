#pragma once
#include "command/command.hpp"
#include "klib/args/parse_result.hpp"
#include <memory>
#include <string_view>

namespace vifo::cli {
class App {
  public:
	[[nodiscard]] auto run(int argc, char const* const* argv) -> int;

  private:
	[[nodiscard]] auto parse_args(int argc, char const* const* argv) -> klib::args::ParseResult;

	template <std::derived_from<Command> T, typename... Args>
		requires(std::constructible_from<T, Args...>)
	void add_command(Args&&... args) {
		m_commands.push_back(std::make_unique<T>(std::forward<Args>(args)...));
		m_commands.back()->populate_args();
	}

	void set_omdb_token();

	std::string m_omdb_token{};
	std::string_view m_title{};

	std::vector<std::unique_ptr<Command>> m_commands{};
};
} // namespace vifo::cli
