#pragma once
#include "vifo/formatter.hpp"
#include "vifo/manifest.hpp"
#include "vifo/result.hpp"
#include <string_view>

namespace vifo {
class DirectoryRenamer {
  public:
	[[nodiscard]] static auto create(std::string_view input_expression, std::string_view output_expression) -> Result<DirectoryRenamer>;

	[[nodiscard]] auto build_manifest(fs::path root) const -> Manifest;

  private:
	std::unique_ptr<IFormatter> m_interpolator{};
};
} // namespace vifo
