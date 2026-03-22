#pragma once
#include "vifo/formatter/pattern_swap_formatter.hpp"
#include "vifo/generator/generator.hpp"
#include <optional>

namespace vifo {
class PatternSwapGenerator : public IGenerator {
  public:
	using Format = PatternSwapFormat;

	[[nodiscard]] static auto create(Format directory, std::optional<Format> file = {}) -> Result<PatternSwapGenerator>;

	[[nodiscard]] auto generate_manifest(fs::path const& directory) -> Manifest final;

  private:
	[[nodiscard]] auto create_directory(Format format) -> Result<void>;
	[[nodiscard]] auto create_file(std::optional<Format> format) -> Result<void>;

	PatternSwapFormatter m_directory{};
	std::optional<PatternSwapFormatter> m_file{};
};
} // namespace vifo
