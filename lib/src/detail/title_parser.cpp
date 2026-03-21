#include "detail/title_parser.hpp"
#include "vifo/panic.hpp"
#include "vifo/util/util.hpp"
#include <regex>
#include <string_view>

namespace vifo::detail {
namespace {
[[nodiscard]] auto is_metadata(std::string_view const word) {
	auto const year = util::to_int(word);
	if (year > 1000 && year < 4000) { return true; }

	static auto const s_resolution_regex = std::regex{R"(\d{3}\d?p)"};
	return std::regex_match(word.data(), word.data() + word.size(), s_resolution_regex);
}
} // namespace

auto TitleParser::parse_and_trim(std::string_view& out_text) -> std::string {
	m_title.clear();
	m_bracket_depth = 0;

	auto scanner = util::WordScanner{out_text};
	auto token = util::WordToken{};
	while (scanner.next(token)) {
		if (!parse(token)) { break; }
	}

	out_text = scanner.get_remain();
	return std::move(m_title);
}

auto TitleParser::parse(util::WordToken const& token) -> bool {
	switch (token.type) {
	case Type::BracketOpen: ++m_bracket_depth; return true;
	case Type::BracketClose: m_bracket_depth = std::max(m_bracket_depth - 1, 0); return true;
	case Type::Word: break;
	default: throw Panic{"internal error: unexpected WordToken::Type"};
	}

	auto const word = token.lexeme;
	if (m_bracket_depth > 0 || is_metadata(word)) { return m_title.empty(); }

	if (word == "-") { return true; }

	util::join_to(m_title, word);

	return true;
}
} // namespace vifo::detail
