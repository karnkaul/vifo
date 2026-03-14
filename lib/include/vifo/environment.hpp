#pragma once
#include "vifo/identifier.hpp"
#include "vifo/result.hpp"
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace vifo {
class Environment {
  public:
	[[nodiscard]] static auto create(std::string_view input_format) -> Result<Environment>;

	[[nodiscard]] auto interpolate(std::string_view input, std::string_view output_format) const -> Result<std::string>;

	[[nodiscard]] auto get_identifiers() const -> std::span<std::unique_ptr<Identifier const> const> { return m_identifiers; }

  private:
	std::string_view m_input_format{};
	std::vector<std::unique_ptr<Identifier const>> m_identifiers{};
};
} // namespace vifo
