#include "vifo/util/regex_matcher.hpp"

namespace vifo::util {
RegexMatcher::RegexMatcher(std::string_view const pattern) try : m_regex(pattern.data(), pattern.size()) {
} catch (std::regex_error const& e) {}

void RegexMatcher::get_matches_to(std::vector<std::string>& out, std::string_view text) const {
	out.clear();
	auto cmatch = get_cmatch(text);
	if (cmatch.size() < 2) { return; }

	auto const span = std::span{cmatch}.subspan(1);
	out.append_range(span);
}

auto RegexMatcher::get_matches(std::string_view text) const -> std::vector<std::string> {
	auto ret = std::vector<std::string>{};
	get_matches_to(ret, text);
	return ret;
}

auto RegexMatcher::get_match(std::string_view text) const -> std::string {
	auto cmatch = get_cmatch(text);
	if (cmatch.size() < 2) { return {}; }
	return cmatch[1]; // NOLINT(cppcoreguidelines-pro-bounds-avoid-unchecked-container-access)
}

auto RegexMatcher::get_cmatch(std::string_view const text) const -> std::cmatch {
	auto ret = std::cmatch{};
	auto const* text_end = text.data() + text.size();
	if (!std::regex_match(text.data(), text_end, ret, m_regex)) { return {}; }
	return ret;
}
} // namespace vifo::util
