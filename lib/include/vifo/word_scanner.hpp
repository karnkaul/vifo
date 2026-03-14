#pragma once
#include <algorithm>
#include <array>
#include <cstdint>
#include <string_view>

namespace vifo {
struct WordToken {
	enum class Type : std::int8_t { Word, BracketOpen, BracketClose, Phrase };

	std::string_view lexeme{};
	Type type{};
};

class WordScanner {
  public:
	explicit constexpr WordScanner(std::string_view const title) : m_remain(title) {}

	[[nodiscard]] constexpr auto next(WordToken& out) -> bool {
		skip_whitespace();
		if (m_remain.empty()) { return false; }
		if (scan_bracket(out)) { return true; }
		out = scan_word();
		return true;
	}

	[[nodiscard]] constexpr auto get_remain() const -> std::string_view { return m_remain; }

  private:
	using Type = WordToken::Type;

	static constexpr auto is_whitespace(char const ch) -> bool {
		constexpr auto whitespace_v = std::array{' ', '\t', '.', '_'};
		return std::ranges::find(whitespace_v, ch) != whitespace_v.end();
	}

	static constexpr auto is_open_bracket(char const ch) -> bool {
		constexpr auto open_bracket_v = std::array{'(', '['};
		return std::ranges::find(open_bracket_v, ch) != open_bracket_v.end();
	}

	static constexpr auto is_close_bracket(char const ch) -> bool {
		constexpr auto close_bracket_v = std::array{')', ']'};
		return std::ranges::find(close_bracket_v, ch) != close_bracket_v.end();
	}

	static constexpr auto is_open_phrase(char const ch) -> bool { return ch == '{'; }
	static constexpr auto is_close_phrase(char const ch) -> bool { return ch == '}'; }

	static constexpr auto is_bracket(char const ch) -> bool { return is_open_bracket(ch) || is_close_bracket(ch); }

	constexpr void skip_whitespace() {
		while (!m_remain.empty() && is_whitespace(m_remain.front())) { m_remain.remove_prefix(1); }
	}

	constexpr auto to_token(Type const type, std::size_t const length) -> WordToken {
		auto const ret = WordToken{.lexeme = m_remain.substr(0, length), .type = type};
		m_remain.remove_prefix(length);
		return ret;
	}

	constexpr auto scan_bracket(WordToken& out) -> bool {
		auto const ch = m_remain.front();
		if (is_open_bracket(ch)) {
			out = to_token(Type::BracketOpen, 1);
			return true;
		}
		if (is_close_bracket(ch)) {
			out = to_token(Type::BracketClose, 1);
			return true;
		}
		return false;
	}

	constexpr auto scan_phrase(WordToken& out) -> bool {
		if (!is_open_phrase(m_remain.front())) { return false; }
		m_remain.remove_prefix(1);
		auto length = 1uz;
		for (; length < m_remain.size(); ++length) {
			if (is_close_phrase(m_remain.at(length))) { break; }
		}
		out = to_token(Type::Phrase, length - 1);
		m_remain.remove_prefix(1);
		return true;
	}

	constexpr auto scan_word() -> WordToken {
		auto length = 1uz;
		for (; length < m_remain.size() && !is_whitespace(m_remain.at(length)); ++length) {
			char const ch = m_remain.at(length);
			if (is_whitespace(ch) || is_bracket(ch)) { break; }
		}
		return to_token(Type::Word, length);
	}

	std::string_view m_remain{};
};
} // namespace vifo
