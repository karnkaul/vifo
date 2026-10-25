#include "detail/title_parser.hpp"
#include "vifo/panic.hpp"
#include "vifo/util/util.hpp"

namespace vifo::detail {
auto TitleParser::parse(std::string_view const text) -> std::string {
	m_title.clear();
	m_bracket_depth = 0;

	auto scanner = util::WordScanner{text};
	auto token = util::WordToken{};
	while (scanner.next(token)) {
		if (!parse(token)) { break; }
	}

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
	if (m_bracket_depth > 0 || util::ignore_for_title(word)) { return m_title.empty(); }

	if (word == "-") { return true; }

	util::join_to(m_title, word);

	return true;
}
} // namespace vifo::detail
