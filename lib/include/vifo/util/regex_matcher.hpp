#pragma once
#include <regex>

namespace vifo::util {
class RegexMatcher {
  public:
	RegexMatcher() = default;

	explicit RegexMatcher(std::string_view pattern);

	void get_matches_to(std::vector<std::string>& out, std::string_view text) const;
	[[nodiscard]] auto get_matches(std::string_view text) const -> std::vector<std::string>;
	[[nodiscard]] auto get_match(std::string_view text) const -> std::string;

  private:
	[[nodiscard]] auto get_cmatch(std::string_view text) const -> std::cmatch;

	std::regex m_regex{};
};
} // namespace vifo::util
