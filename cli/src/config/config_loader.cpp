#include "config/config_loader.hpp"
#include "klib/env.hpp"
#include "log.hpp"
#include "vifo/json_io.hpp"
#include <array>

namespace vifo::cli {
namespace {
[[nodiscard]] auto get_default_config_directory() {
	static constexpr auto xdg_config_home_key_v = klib::CString{"XDG_CONFIG_HOME"};
	auto const xdg_config_home = klib::env::get_var(xdg_config_home_key_v);
	if (!xdg_config_home.as_view().empty()) { return fs::path{xdg_config_home.as_view()}; }

	auto home = fs::path{klib::env::get_var("HOME").as_view()};
	if (home.empty()) { home = fs::current_path(); }

	auto ret = home / ".config";
	if (!fs::exists(ret)) {
		auto err = std::error_code{};
		if (!fs::create_directories(ret, err)) { ret = std::move(home); }
	}

	return ret;
}
} // namespace

auto ConfigLoader::try_load(fs::path const& custom_path) -> bool {
	if (!try_load_json(custom_path)) { return false; }

	populate_config();
	log.info("Config loaded from: {}", m_source.generic_string());

	return true;
}

auto ConfigLoader::try_load_json(fs::path const& path) -> bool {
	if (!path.empty()) {
		if (load_json(path)) { return true; }
		log.warn("Failed to load custom config from: {}", path.generic_string());
	}

	static constexpr auto default_filenames_v = std::array{
		"vifo_config.jsonc",
		"vifo_config.json",
	};

	auto const load_any = [this](fs::path const& parent) {
		return std::ranges::any_of(default_filenames_v, [&](std::string_view const filename) { return load_json(parent / filename); });
	};

	auto parent = get_default_config_directory();
	if (load_any(parent)) { return true; }
	parent = fs::current_path();
	if (load_any(parent)) { return true; }

	log.warn("Failed to load default config");
	return false;
}

auto ConfigLoader::load_json(fs::path path) -> bool {
	auto result = dj::Json::from_file(path.generic_string());
	if (!result) { return false; }

	m_source = std::move(path);
	m_json = std::move(*result);
	return true;
}

void ConfigLoader::populate_config() {
	if (auto const& omdb_api_token = m_json["omdb_api_token"]) { from_json(omdb_api_token, config.omdb_api_token); }
	if (auto const& formats = m_json["formats"]) { from_json(formats, config.formats); }
}
} // namespace vifo::cli
