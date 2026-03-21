#pragma once
#include "klib/base_types.hpp"
#include "vifo/environment.hpp"
#include "vifo/expression.hpp"
#include <filesystem>
#include <memory>
#include <string>
#include <string_view>

namespace vifo {
namespace fs = std::filesystem;

class Formatter : public klib::Polymorphic {
  public:
	/// \returns Transformed string on match, otherwise empty string.
	[[nodiscard]] virtual auto format_string(std::string_view input) -> std::string = 0;

	/// \returns Path with transformed stem on match, otherwise empty path.
	[[nodiscard]] auto format_path(fs::path const& path) -> fs::path;

  protected:
	[[nodiscard]] auto interpolate(expression::Expression const& expression) const -> std::string;

	std::unique_ptr<Environment> m_environment{std::make_unique<Environment>()};
};
} // namespace vifo
