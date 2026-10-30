#include "command/omdb.hpp"
#include "klib/debug/assert.hpp"
#include "state/format.hpp"
#include "vifo/types.hpp"
#include "vifo/util/util.hpp"
#include <format>

namespace vifo::cli::command {
OmdbBase::OmdbBase(omdb::IService const& omdb_service, GetConfig get_config, std::string_view name)
	: m_omdb_service(&omdb_service), m_get_config(std::move(get_config)), m_command_name(name) {
	m_command_help = std::format("format a {} directory", m_command_name);
	m_directory_help = std::format("path to {} directory (default = .)", m_command_name);
}

auto OmdbBase::get_parameters() -> std::vector<clap::Parameter> {
	return {
		clap::named_option(m_title_override, "t,title-override", "title override"),
		clap::positional_optional(m_directory, "DIR", m_directory_help),
	};
}

auto OmdbBase::execute() -> ExitCode {
	auto directory = util::path_if_directory(m_directory);
	if (directory.empty()) { return ExitCode::InvalidArgument; }

	auto const result = create_formatter(m_get_config());
	if (result != ExitCode::Success) { return result; }
	KLIB_ASSERT(m_formatter);
	m_formatter->search_title_override = m_title_override;

	return execute_state_machine(std::make_unique<StateFormat>(*m_formatter, std::move(directory)));
}
} // namespace vifo::cli::command
