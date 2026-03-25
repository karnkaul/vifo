#pragma once
#include "command/command.hpp"
#include "klib/ptr.hpp"
#include "vifo/omdb.hpp"
#include <string_view>

namespace vifo::cli::command {
class Movie : public Command {
  public:
	explicit Movie(omdb::IService const& omdb_service) : m_omdb_service(&omdb_service) {}

  private:
	static constexpr std::string_view name_v{"movie"};
	static constexpr std::string_view help_v{"format a movie directory"};

	[[nodiscard]] auto get_name() const -> std::string_view final { return name_v; }
	[[nodiscard]] auto get_help() const -> std::string_view final { return help_v; }

	void populate_args() final;

	[[nodiscard]] auto execute() -> ExitCode final;

	klib::Ptr<omdb::IService const> m_omdb_service{};

	std::string_view m_directory{};
};
} // namespace vifo::cli::command
