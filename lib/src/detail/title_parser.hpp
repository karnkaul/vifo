#pragma once
#include "vifo/util/word_scanner.hpp"
#include <string>
#include <string_view>

namespace vifo::detail {
class TitleParser {
  public:
	[[nodiscard]] auto parse(std::string_view text) -> std::string;

  private:
	using Type = util::WordToken::Type;

	auto parse(util::WordToken const& token) -> bool;

	std::string m_title{};
	int m_bracket_depth{0};
};
} // namespace vifo::detail
