#include "detail/identifier.hpp"
#include "detail/title_parser.hpp"
#include "vifo/error.hpp"
#include "vifo/result.hpp"
#include "vifo/util.hpp"
#include <string_view>

namespace vifo::detail {
namespace {
template <typename Type>
[[nodiscard]] auto create_builtin(std::string_view const name) -> std::unique_ptr<Identifier> {
	if (Type::name_v == name) { return std::make_unique<Type>(); }
	return {};
}
} // namespace

auto Identifier::try_strip_input(std::string_view& out_text) -> Result<std::string_view> {
	if (out_text.size() < 2 || out_text.front() != open_v) { return {}; }

	auto const close_idx = out_text.find(Identifier::close_v);
	if (close_idx == std::string_view::npos) {
		return to_error(Error::Type::SyntaxError, std::format("missing identifier close ({}): '{}'", Identifier::close_v, out_text));
	}
	if (close_idx == 1) { return to_error(Error::Type::SyntaxError, std::format("missing identifier name: '{}'", out_text)); }

	auto ret = out_text.substr(0, close_idx + 1);
	out_text = out_text.substr(close_idx + 1);
	return ret;
}

auto Identifier::parse_input(std::string_view text) -> Result<Input> {
	if (text.size() < 2 || text.front() != open_v || text.back() != close_v) {
		return to_error(Error::Type::SyntaxError, std::format("not an identifier: '{}'", text));
	}

	text.remove_prefix(1);
	text.remove_suffix(1);

	if (text.empty()) { return to_error(Error::Type::SyntaxError, "identifier missing name"); }

	auto ret = Input{};
	auto const delim_idx = text.find(delim_v);
	if (delim_idx == std::string_view::npos) {
		ret.name = text;
		return ret;
	}

	ret.name = text.substr(0, delim_idx);

	auto const length_text = text.substr(delim_idx + 1);
	auto const i_length = util::to_int(length_text);
	if (i_length < 0) { return to_error(Error::Type::SyntaxError, std::format("invalid length: '{}'", length_text)); }

	ret.max_length = std::size_t(i_length);

	return ret;
}

auto Identifier::create(Input const input) -> std::unique_ptr<Identifier> {
	if (auto ret = create_builtin<Year>(input.name)) { return ret; }
	if (auto ret = create_builtin<Title>(input.name)) { return ret; }
	return std::make_unique<Identifier>(std::string{input.name}, input.max_length);
}

auto Identifier::parse_value(std::string_view& out_text) -> bool {
	auto const length = [&] {
		if (m_max_length == 0) { return out_text.length(); }
		return std::min(m_max_length, out_text.length());
	}();

	m_value = out_text.substr(0, length);
	out_text.remove_prefix(length);

	return true;
}

auto Year::parse_value(std::string_view& out_text) -> bool {
	if (out_text.size() < 4) { return false; }

	m_value = out_text.substr(0, 4);
	auto const value = util::to_int(m_value);
	if (value < 1000) { return false; }

	out_text.remove_prefix(m_value.size());
	return true;
}

auto Title::parse_value(std::string_view& out_text) -> bool {
	auto parser = detail::TitleParser{};
	m_value = parser.parse_and_trim(out_text);
	return !m_value.empty();
}
} // namespace vifo::detail
