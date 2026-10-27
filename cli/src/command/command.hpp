#pragma once
#include "clap/parameter.hpp"
#include "config/config.hpp"
#include "klib/base_types.hpp"
#include "vifo/types.hpp"
#include <functional>
#include <print>
#include <string_view>
#include <vector>

namespace vifo::cli {
using GetConfig = std::move_only_function<Config const&()>;

class Command : public klib::Polymorphic, public klib::Pinned {
  public:
	[[nodiscard]] virtual auto get_name() const -> std::string_view = 0;
	[[nodiscard]] virtual auto get_help() const -> std::string_view { return {}; }

	virtual auto get_parameters() -> std::vector<clap::Parameter> { return {}; }

	[[nodiscard]] virtual auto execute() -> ExitCode = 0;

  protected:
	[[nodiscard]] static auto handle_error(Error const& error) -> ExitCode {
		std::println("{}", error.message);
		return to_exit_code(error.type);
	}
};
} // namespace vifo::cli
