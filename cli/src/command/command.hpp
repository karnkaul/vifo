#pragma once
#include "klib/args/arg.hpp"
#include "klib/base_types.hpp"
#include "vifo/exit_code.hpp"
#include <span>
#include <string_view>
#include <vector>

namespace vifo::cli {
class Command : public klib::Polymorphic, public klib::Pinned {
  public:
	[[nodiscard]] virtual auto get_name() const -> std::string_view = 0;
	[[nodiscard]] virtual auto get_help() const -> std::string_view { return {}; }

	virtual void populate_args() {}

	[[nodiscard]] virtual auto execute() -> ExitCode = 0;

	[[nodiscard]] auto get_args() const -> std::span<klib::args::Arg const> { return m_args; }

  protected:
	std::vector<klib::args::Arg> m_args{};
};
} // namespace vifo::cli
