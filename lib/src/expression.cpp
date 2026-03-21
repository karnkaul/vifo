#include "vifo/expression.hpp"
#include "detail/common.hpp"
#include "klib/debug/assert.hpp"
#include "klib/ptr.hpp"
#include "klib/visitor.hpp"
#include "vifo/util/util.hpp"
#include <cstddef>
#include <string>
#include <string_view>

namespace vifo {
namespace expression {
namespace {
class Tokenizer {
  public:
	explicit constexpr Tokenizer(std::string_view const input) : m_text(input) {}

	constexpr auto next(Token& out) -> bool {
		if (at_end()) { return false; }
		switch (m_text.at(m_index)) {
		case '{': out = to_token(Token::Type::BraceLeft, 1); break;
		case '}': out = to_token(Token::Type::BraceRight, 1); break;
		default: out = scan_substring(); break;
		}
		return true;
	}

  private:
	[[nodiscard]] constexpr auto at_end() const -> bool { return m_index >= m_text.size(); }

	[[nodiscard]] constexpr auto to_token(Token::Type type, std::size_t const length) -> Token {
		auto const ret = Token{.type = type, .start_index = m_index, .length = length};
		m_index += length;
		return ret;
	}

	[[nodiscard]] constexpr auto scan_substring() -> Token {
		auto text = m_text.substr(m_index);
		auto const i = text.find_first_of("{}");
		if (i == std::string_view::npos) { return to_token(Token::Type::Substring, text.size()); }
		return to_token(Token::Type::Substring, i);
	}

	std::string_view m_text{};

	std::size_t m_index{};
};

[[nodiscard]] auto tokenize(std::string_view const input) -> std::vector<Token> {
	auto ret = std::vector<Token>{};
	auto token = Token{};
	auto tokenizer = Tokenizer{input};
	while (tokenizer.next(token)) { ret.push_back(token); }
	return ret;
}

class Parser {
  public:
	[[nodiscard]] auto parse(std::string_view const input) -> Result<Expression> {
		m_input = input;
		m_tokens = tokenize(m_input);
		m_index = 0;

		auto ret = Expression{};
		while (!at_end()) {
			switch (current().type) {
			case Token::Type::Substring: {
				auto atom = Atom{
					.value = Substring{.text = std::string(current().get_lexeme(m_input))},
					.token = current(),
				};
				ret.atoms.push_back(std::move(atom));
				advance();
				break;
			}
			case Token::Type::BraceLeft: {
				auto id_token = Token{};
				auto identifier = parse_identifier(id_token);
				if (!identifier) { return std::unexpected{identifier.error()}; }
				ret.atoms.push_back(Atom{.value = std::move(*identifier), .token = id_token});
				break;
			}
			case Token::Type::BraceRight: return syntax_error(current(), "unexpected '}'");
			default: return syntax_error(current(), "ICE");
			}
		}

		return ret;
	}

  private:
	[[nodiscard]] auto syntax_error(Token const token, std::string_view const msg) const -> std::unexpected<Error> {
		return detail::to_error(Error::Type::Syntax, token, m_input, msg);
	}

	[[nodiscard]] auto create_identifier(Token const token) const -> Result<Identifier> {
		auto const lexeme = token.get_lexeme(m_input);
		auto name = lexeme;
		auto length = 0uz;
		if (auto const i = name.find(':'); i != std::string_view::npos) {
			if (i == 0) { return syntax_error(token, "missing Identifier name"); }
			if (i + 1 == name.size()) { return syntax_error(token, "missing length"); }

			name = lexeme.substr(0, i);
			auto const length_str = lexeme.substr(i + 1);
			auto const i_length = util::to_int(length_str, -1);
			if (i_length < 0) { return syntax_error(token, std::format("invalid length: {}", length_str)); }

			length = std::size_t(i_length);
		}

		return Identifier{.name = std::string{name}, .length = length};
	}

	[[nodiscard]] auto at_end() const -> bool { return m_index >= m_tokens.size(); }

	[[nodiscard]] auto current() const -> Token const& {
		KLIB_ASSERT(!at_end());
		return m_tokens[m_index];
	}

	[[nodiscard]] auto peek() const -> klib::Ptr<Token const> {
		auto const index = m_index + 1;
		if (index >= m_tokens.size()) { return nullptr; }
		return &m_tokens[index];
	}

	void advance() {
		if (at_end()) { return; }
		++m_index;
	}

	[[nodiscard]] auto parse_identifier(Token& out_id) -> Result<Identifier> {
		auto const brace_open = current();
		advance();
		if (at_end()) { return syntax_error(brace_open, "incomplete identifier"); }

		out_id = current();
		advance();
		if (at_end()) { return syntax_error(brace_open, "incomplete identifier"); }
		if (current().type != Token::Type::BraceRight) { return syntax_error(brace_open, "missing '}'"); }
		advance();

		return create_identifier(out_id);
	}

	std::string_view m_input{};
	std::vector<Token> m_tokens{};
	std::size_t m_index{};
};
} // namespace

auto Token::get_lexeme(std::string_view const text) const -> std::string_view {
	KLIB_ASSERT(start_index + length <= text.size());
	return text.substr(start_index, length);
}

auto Substring::consume(std::string_view& out_input) const -> bool {
	if (!out_input.starts_with(text)) { return false; }
	out_input.remove_prefix(text.size());
	return true;
}

auto Expression::interpolate(GetValue get_value) const -> std::string {
	if (!get_value) { return {}; }
	auto ret = std::string{};
	auto const visitor = klib::Visitor{
		[&](Substring const& substring) { ret += substring.text; },
		[&](Identifier const& identifier) { ret += get_value(identifier); },
	};
	for (auto const& atom : atoms) { std::visit(visitor, atom.value); }
	return ret;
}
} // namespace expression

auto expression::parse(std::string_view input) -> Result<Expression> {
	auto parser = Parser{};
	return parser.parse(input);
}
} // namespace vifo
