#include "vifo/expression.hpp"
#include "detail/common.hpp"
#include "klib/lerp_expr/scanner.hpp"
#include "vifo/util/util.hpp"
#include <cstddef>
#include <string>
#include <string_view>

namespace vifo {
namespace expression {
namespace {
class Parser {
  public:
	[[nodiscard]] auto parse(std::string_view const input) -> Result<Expression> {
		m_input = input;

		auto ret = Expression{};

		auto const per_token = [&](Token const& token) {
			switch (token.type) {
			case Token::Type::String: {
				ret.atoms.push_back(Atom{
					.value = Substring{.text = std::string{token.lexeme}},
					.token = token,
				});
				break;
			}
			case Token::Type::Identifier: {
				auto identifier = create_identifier(token);
				if (!identifier) {
					m_error = std::move(identifier.error());
					throw StopParsing{};
				}
				ret.atoms.push_back(Atom{
					.value = std::move(*identifier),
					.token = token,
				});
				break;
			}
			default: break;
			}
		};

		try {
			klib::lerp_expr::tokenize(m_input, per_token);
		} catch (StopParsing) { return std::unexpected{std::move(m_error)}; }

		return ret;
	}

  private:
	struct StopParsing {};

	[[nodiscard]] auto syntax_error(Token const token, std::string_view const msg) const -> std::unexpected<Error> {
		return detail::to_error(Error::Type::Syntax, token, m_input, msg);
	}

	[[nodiscard]] auto create_identifier(Token const token) const -> Result<Identifier> {
		auto name = token.lexeme;
		auto length = 0uz;
		if (auto const i = name.find(':'); i != std::string_view::npos) {
			if (i == 0) { return syntax_error(token, "missing Identifier name"); }
			if (i + 1 == name.size()) { return syntax_error(token, "missing length"); }

			name = token.lexeme.substr(0, i);
			auto const length_str = token.lexeme.substr(i + 1);
			auto const i_length = util::to_int(length_str, -1);
			if (i_length < 0) { return syntax_error(token, std::format("invalid length: {}", length_str)); }

			length = std::size_t(i_length);
		}

		return Identifier{.name = std::string{name}, .length = length};
	}

	std::string_view m_input;

	Error m_error{};
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
