#pragma once
#include "vifo/formatter/formatter.hpp"
#include "vifo/interpolator/pattern_swap.hpp"

namespace vifo::formatter {
class PatternSwap : public IFormatter {
  public:
	using Format = interpolator::PatternSwapFormat;

	[[nodiscard]] static auto create(Format const& directory, std::optional<Format> file = {}) -> Result<PatternSwap>;

	[[nodiscard]] auto generate_manifest(fs::path const& directory) -> Result<Manifest> final;

  private:
	[[nodiscard]] auto create_directory(Format const& format) -> Result<void>;
	[[nodiscard]] auto create_file(std::optional<Format> const& format) -> Result<void>;

	interpolator::PatternSwap m_directory{};
	std::optional<interpolator::PatternSwap> m_file{};
};
} // namespace vifo::formatter
