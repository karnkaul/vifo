#pragma once
#include "command/command.hpp"
#include "klib/ptr.hpp"
#include "vifo/formatter/formatter.hpp"
#include "vifo/formatter/movie.hpp"
#include "vifo/formatter/season.hpp"
#include "vifo/formatter/series.hpp"
#include "vifo/omdb.hpp"
#include "vifo/types.hpp"
#include "vifo/util/util.hpp"
#include <string_view>

namespace vifo::cli::command {
class OmdbBase : public Command {
  public:
	explicit OmdbBase(omdb::IService const& omdb_service, GetConfig get_config, std::string_view name);

  protected:
	virtual auto create_formatter(Config const& config) -> ExitCode = 0;

	klib::Ptr<omdb::IService const> m_omdb_service{};
	std::unique_ptr<formatter::Omdb> m_formatter{};

  private:
	[[nodiscard]] auto get_name() const -> std::string_view final { return m_command_name; }
	[[nodiscard]] auto get_help() const -> std::string_view final { return m_command_help; }
	auto get_parameters() -> std::vector<clap::Parameter> final;
	auto execute() -> ExitCode final;

	GetConfig m_get_config{};
	std::string_view m_command_name{};
	std::string m_command_help{};
	std::string m_directory_help{};

	std::string_view m_title_override{};
	std::string_view m_directory{"."};
};

template <std::derived_from<IFormatter> FormatterT>
class Omdb : public OmdbBase {
  public:
	explicit Omdb(omdb::IService const& omdb_service, GetConfig get_config) : OmdbBase(omdb_service, std::move(get_config), util::get_name_for<FormatterT>()) {}

  private:
	auto create_formatter(Config const& config) -> ExitCode final {
		auto formatter = FormatterT::create(*m_omdb_service, util::get_format_for<FormatterT>(config.formats));
		if (!formatter) { return handle_error(formatter.error()); }
		m_formatter = std::make_unique<FormatterT>(std::move(*formatter));
		return ExitCode::Success;
	}
};

using Movie = Omdb<formatter::Movie>;
using Season = Omdb<formatter::Season>;
using Series = Omdb<formatter::Series>;
} // namespace vifo::cli::command
