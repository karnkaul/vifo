#pragma once
#include "vifo/formatter.hpp"
#include "vifo/manifest.hpp"
#include "vifo/result.hpp"
#include <string_view>

namespace vifo {
class DirectoryRenamer {
  public:
	[[nodiscard]] static auto create_interpolator(std::string_view input_expression, std::string_view output_expression) -> Result<DirectoryRenamer>;

	explicit DirectoryRenamer(std::unique_ptr<IFormatter> formatter) : m_formatter(std::move(formatter)) {}

	[[nodiscard]] auto build_manifest(fs::path const& root) const -> Manifest;

  private:
	std::unique_ptr<IFormatter> m_formatter{};
};
} // namespace vifo
