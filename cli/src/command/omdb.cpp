#include "command/omdb.hpp"
#include "djson/json.hpp"
#include "klib/debug/assert.hpp"
#include "log.hpp"
#include "state/format.hpp"
#include "vifo/types.hpp"
#include "vifo/util/util.hpp"
#include <format>

namespace vifo::cli::command {
OmdbBase::OmdbBase(omdb::IService const& omdb_service, std::string_view name) : m_omdb_service(&omdb_service), m_command_name(name) {
	m_command_help = std::format("format a {} directory", m_command_name);
	m_json_help = std::format("path to json specifying {} format", m_command_name);
	m_directory_help = std::format("path to {} directory (default = .)", m_command_name);
}

auto OmdbBase::get_parameters() -> std::vector<clap::Parameter> {
	return {
		clap::named_option(m_format_json, "f,format-json", m_json_help),
		clap::positional_optional(m_directory, "DIR", m_directory_help),
	};
}

auto OmdbBase::execute() -> ExitCode {
	auto directory = util::path_if_directory(m_directory);
	if (directory.empty()) { return ExitCode::InvalidArgument; }

	if (!m_format_json.empty()) {
		auto json = dj::Json::from_file(m_format_json);
		if (!json) {
			std::println(stderr, "failed to read format json: '{}'", m_format_json);
			return ExitCode::IoError;
		}
		m_json = std::move(*json);
		read_from_json();
		auto const serialized = m_json.serialize({.flags = dj::SerializeFlag::NoSpaces});
		log.info("read {} format from json: {}\n{}", m_command_name, m_format_json, serialized);
	}

	auto const result = create_formatter();
	if (result != ExitCode::Success) { return result; }
	KLIB_ASSERT(m_formatter);

	return execute_state_machine(std::make_unique<StateFormat>(*m_formatter, std::move(directory)));
}
} // namespace vifo::cli::command
