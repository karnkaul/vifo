#pragma once
#include "vifo/util/word_scanner.hpp"
#include <string>
#include <string_view>

namespace vifo::detail {
class TitleParser {
  public:
	[[nodiscard]] auto parse(std::string_view text) -> std::string { return parse_and_trim(text); }
	[[nodiscard]] auto parse_and_trim(std::string_view& out_text) -> std::string;

  private:
	using Type = util::WordToken::Type;

	auto parse(util::WordToken const& token) -> bool;

	std::string m_title{};
	int m_bracket_depth{0};
};
} // namespace vifo::detail
