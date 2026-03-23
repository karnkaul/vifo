#include "app/app.hpp"
#include "command/command.hpp"
#include "command/ghost_copy.hpp"
#include "command/patswap.hpp"
#include "klib/args/arg.hpp"
#include "klib/args/parse.hpp"
#include "klib/args/parse_info.hpp"
#include "klib/env.hpp"
#include "klib/string/c_string.hpp"
#include "log.hpp"
#include "vifo/build_version.hpp"
#include "vifo/omdb.hpp"
#include "vifo/panic.hpp"
#include <cstdlib>
#include <string>
#include <string_view>

namespace vifo::cli {
namespace {
constexpr auto env_omdb_token_key_v = klib::CString{"VIFO_OMDB_TOKEN"};
} // namespace

auto App::run(int argc, char const* const* argv) -> int {
	auto const get_api_token = [this] -> std::string_view {
		set_omdb_token();
		return m_omdb_token;
	};

	auto const omdb_service = omdb::IService::create(get_api_token, omdb::Curl::Internal);

	add_command<command::Patswap>();
	add_command<command::GhostCopy>();

	auto const parse_result = parse_args(argc, argv);
	if (parse_result.early_return()) { return parse_result.get_return_code(); }

	log.debug("{}", build_version_v);

	auto const it = std::ranges::find_if(m_commands, [name = parse_result.get_command_name()](auto const& c) { return c->get_name() == name; });
	if (it == m_commands.end()) {
		log.error("unrecognized command: {}", parse_result.get_command_name());
		return EXIT_FAILURE;
	}

	auto& command = **it;

	return int(command.execute());
}

auto App::parse_args(int argc, char const* const* argv) -> klib::args::ParseResult {
	auto const parse_info = klib::args::ParseInfo{
		.version = build_version_str,
	};
	auto args = std::vector{
		klib::args::named_option(m_omdb_token, "o,omdb-token", "omdb API token"),
	};
	for (auto const& command : m_commands) { args.push_back(klib::args::command(command->get_args(), command->get_name(), command->get_help())); }
	return klib::args::parse_main(parse_info, args, argc, argv);
}

void App::set_omdb_token() {
	if (!m_omdb_token.empty()) { return; }

	log.debug("reading omdb API token from env:{}", env_omdb_token_key_v.as_view());
	auto const omdb_token = klib::env::get_var(env_omdb_token_key_v);
	if (omdb_token.as_view().empty()) { throw Panic{"invalid (empty) ombd API token"}; }
	m_omdb_token = omdb_token.as_view();
}
} // namespace vifo::cli
