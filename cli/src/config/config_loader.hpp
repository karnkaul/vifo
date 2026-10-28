#pragma once
#include "config/config.hpp"
#include <djson/json.hpp>
#include <filesystem>

namespace vifo::cli {
namespace fs = std::filesystem;

class ConfigLoader {
  public:
	[[nodiscard]] auto get_source() const -> fs::path const& { return m_source; }
	auto try_load(fs::path const& custom_path = {}) -> bool;

	Config config{};

  private:
	auto try_load_json(fs::path const& path) -> bool;
	auto load_json(fs::path path) -> bool;
	void populate_config();

	dj::Json m_json{};
	fs::path m_source{};
};
} // namespace vifo::cli
