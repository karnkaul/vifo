#pragma once
#include "vifo/formatter/formatter.hpp"
#include "vifo/interpolator/pattern_swap.hpp"
#include <optional>

namespace vifo::formatter {
class PatternSwap : public IFormatter {
  public:
	struct Format {
		interpolator::PatternSwapFormat directory{};
		std::optional<interpolator::PatternSwapFormat> file{};
	};

	[[nodiscard]] static auto create(Format const& format) -> Result<PatternSwap>;

	[[nodiscard]] auto generate_manifest(fs::path const& directory) -> Result<Manifest> final;

  private:
	[[nodiscard]] auto create_directory(interpolator::PatternSwapFormat const& format) -> Result<void>;
	[[nodiscard]] auto create_file(std::optional<interpolator::PatternSwapFormat> const& format) -> Result<void>;

	interpolator::PatternSwap m_directory{};
	std::optional<interpolator::PatternSwap> m_file{};
};
} // namespace vifo::formatter
