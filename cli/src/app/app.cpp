#include "app/app.hpp"
#include "clap/parameter.hpp"
#include "clap/parser.hpp"
#include "clap/spec.hpp"
#include "command/command.hpp"
#include "command/ghost_copy.hpp"
#include "command/omdb.hpp"
#include "command/patswap.hpp"
#include "command/print_metadata.hpp"
#include "klib/env.hpp"
#include "klib/string/c_string.hpp"
#include "log.hpp"
#include "vifo/build_version.hpp"
#include "vifo/omdb.hpp"
#include <cstdlib>

namespace vifo::cli {
namespace {
constexpr auto env_omdb_token_key_v = klib::CString{"VIFO_OMDB_TOKEN"};
} // namespace

auto App::run(int argc, char const* const* argv) -> int {
	auto const get_config = [this] -> Config const& {
		if (!m_config_loaded) { m_config_loaded = m_config_loader.try_load(m_config_path); }
		return m_config_loader.config;
	};

	auto const get_api_token = [&] -> std::string_view {
		get_config();
		set_omdb_token();
		return m_config_loader.config.omdb_api_token;
	};

	auto const omdb_service = omdb::IService::create(get_api_token, omdb::Curl::Internal);

	add_command<command::Patswap>(get_config);
	add_command<command::Movie>(*omdb_service, get_config);
	add_command<command::Season>(*omdb_service, get_config);
	add_command<command::Series>(*omdb_service, get_config);
	add_command<command::GhostCopy>();
	add_command<command::PrintMetadata>();

	auto const parse_result = parse_args(argc, argv);
	if (parse_result.should_early_exit()) { return parse_result.return_code(); }

	log.debug("{}", build_version_v);

	auto const identifier = parse_result.command_identifier();
	auto const it = std::ranges::find_if(m_commands, [identifier](auto const& c) { return c->get_name() == identifier; });
	if (it == m_commands.end()) {
		log.error("unrecognized command: {}", identifier);
		return EXIT_FAILURE;
	}

	auto& command = **it;

	return static_cast<int>(command.execute());
}

auto App::parse_args(int argc, char const* const* argv) -> clap::Result {
	auto spec = clap::spec::Commands{
		.options =
			{
				clap::named_option(m_config_path, "c,config", "path to config JSON"),
			},
		.program =
			{
				.version = build_version_str,
				.description = "video file formatter",
			},
	};
	spec.commands.reserve(m_commands.size());
	for (auto const& command : m_commands) { spec.commands.push_back(clap::command(command->get_name(), command->get_parameters(), command->get_help())); }

	auto parser = clap::Parser{std::move(spec)};
	return parser.parse_main(argc, argv);
}

void App::set_omdb_token() {
	if (!m_config_loader.config.omdb_api_token.empty()) { return; }

	log.debug("reading omdb API token from env:{}", env_omdb_token_key_v.as_view());
	auto const omdb_token = klib::env::get_var(env_omdb_token_key_v);
	m_config_loader.config.omdb_api_token = omdb_token.as_view();
}
} // namespace vifo::cli
