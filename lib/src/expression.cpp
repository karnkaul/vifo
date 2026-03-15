#include "vifo/expression.hpp"
#include "klib/assert.hpp"
#include "klib/base_types.hpp"
#include "klib/enum_name.hpp"
#include "klib/ptr.hpp"
#include "vifo/util.hpp"
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

namespace vifo {
namespace expression {
namespace {
struct Token {
	enum class Type : std::int8_t { Substring, BraceLeft, BraceRight };
	inline static auto const type_name_map = klib::EnumNameMap<Type>{
		{Type::Substring, "Substring"},
		{Type::BraceLeft, "BraceLeft"},
		{Type::BraceRight, "BraceRight"},
	};

	Type type{};
	std::string_view lexeme{};
	std::size_t start_index{};
};

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
		auto const ret = Token{.type = type, .lexeme = m_text.substr(m_index, length), .start_index = m_index};
		m_index += ret.lexeme.size();
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

		while (!at_end()) {
			switch (current().type) {
			case Token::Type::Substring: {
				m_ret.atoms.emplace_back(Substring{.text = std::string{current().lexeme}});
				advance();
				break;
			}
			case Token::Type::BraceLeft: {
				auto ret = parse_identifier();
				if (!ret) { return std::unexpected{ret.error()}; }
				m_ret.atoms.emplace_back(std::move(*ret));
				break;
			}
			case Token::Type::BraceRight: return create_syntax_error(current(), "unexpected '}'");
			default: return create_syntax_error(current(), "ICE");
			}
		}

		return std::move(m_ret);
	}

  private:
	[[nodiscard]] auto create_syntax_error(Token const token, std::string_view const msg) const -> std::unexpected<Error> {
		auto message = std::string{msg};
		std::format_to(std::back_inserter(message), "\n | {}\n | ", m_input);
		for (std::size_t i = 0; i < token.start_index; ++i) { message.push_back(' '); }
		for (std::size_t i = 0; i < token.lexeme.size(); ++i) { message.push_back('^'); }
		return to_error(Error::Type::Syntax, message);
	}

	[[nodiscard]] auto create_identifier(Token const token) const -> Result<Identifier> {
		auto name = token.lexeme;
		auto length = 0uz;
		if (auto const i = name.find(':'); i != std::string_view::npos) {
			if (i == 0) { return create_syntax_error(token, "missing Identifier name"); }
			if (i + 1 == name.size()) { return create_syntax_error(token, "missing length"); }

			name = token.lexeme.substr(0, i);
			auto const length_str = token.lexeme.substr(i + 1);
			auto const i_length = util::to_int(length_str, -1);
			if (i_length < 0) { return create_syntax_error(token, std::format("invalid length: {}", length_str)); }

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

	[[nodiscard]] auto parse_identifier() -> Result<Identifier> {
		auto const brace_open = current();
		advance();
		if (at_end()) { return create_syntax_error(brace_open, "incomplete identifier"); }

		auto const id = current();
		advance();
		if (at_end()) { return create_syntax_error(brace_open, "incomplete identifier"); }
		if (current().type != Token::Type::BraceRight) { return create_syntax_error(brace_open, "missing '}'"); }
		advance();

		return create_identifier(id);
	}

	std::string_view m_input{};
	std::vector<Token> m_tokens{};
	std::size_t m_index{};

	Expression m_ret{};
};

class IIdentifier : public klib::Polymorphic {
  public:
	explicit IIdentifier(std::string name) : m_name(std::move(name)) {}

	virtual auto parse_value(std::string_view& out_input, std::string& out_output) const -> bool = 0;

	[[nodiscard]] auto get_name() const -> std::string_view { return m_name; }

  private:
	std::string m_name{};
};

class Variable : public IIdentifier {
  public:
	explicit Variable(std::string name, std::size_t const max_length) : IIdentifier(std::move(name)), m_max_length(max_length) {}

  private:
	auto parse_value(std::string_view& out_text, std::string& out_output) const -> bool final {
		auto const length = [&] {
			if (m_max_length == 0) { return out_text.length(); }
			return std::min(m_max_length, out_text.length());
		}();

		out_output += out_text.substr(0, length);
		out_text.remove_prefix(length);
		return true;
	}

	std::size_t m_max_length{};
};

class Year : public IIdentifier {
  public:
	static constexpr std::string_view name_v{"year"};

	explicit Year() : IIdentifier(std::string{name_v}) {}

  private:
	[[nodiscard]] auto parse_value(std::string_view& out_input, std::string& out_output) const -> bool final {
		if (out_input.size() < 4) { return false; }

		auto const value = out_input.substr(0, 4);
		auto const i_value = util::to_int(value);
		if (i_value < 1000) { return false; }

		out_input.remove_prefix(value.size());
		out_output += value;
		return true;
	}
};
} // namespace

auto Substring::consume(std::string_view& out_input) const -> bool {
	if (!out_input.starts_with(text)) { return false; }
	out_input.remove_prefix(text.size());
	return true;
}
} // namespace expression

auto expression::parse(std::string_view input) -> Result<Expression> {
	auto parser = Parser{};
	return parser.parse(input);
}
} // namespace vifo
