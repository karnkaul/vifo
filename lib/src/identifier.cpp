#include "vifo/identifier.hpp"
#include "detail/title_parser.hpp"
#include "vifo/error.hpp"
#include "vifo/result.hpp"
#include "vifo/util.hpp"
#include <string_view>

namespace vifo {
namespace {
class Year : public Identifier {
  public:
	static constexpr std::string_view name_v{"year"};

	explicit Year() : Identifier(std::string{name_v}, 4) {}

  private:
	[[nodiscard]] auto parse_value(std::string_view& out_text) const -> std::optional<std::string> final {
		if (out_text.size() < 4) { return {}; }

		auto const value = out_text.substr(0, 4);
		auto const i_value = util::to_int(value);
		if (i_value < 1000) { return {}; }

		out_text.remove_prefix(value.size());
		return std::string{value};
	}
};

class Title : public Identifier {
  public:
	static constexpr std::string_view name_v{"title"};

	explicit Title() : Identifier(std::string{name_v}, 4) {}

  private:
	[[nodiscard]] auto parse_value(std::string_view& out_text) const -> std::optional<std::string> final {
		auto parser = detail::TitleParser{};
		auto ret = parser.parse_and_trim(out_text);
		if (ret.empty()) { return {}; }
		return std::string{ret};
	}
};

template <typename Type>
[[nodiscard]] auto create_builtin(std::string_view const name) -> std::unique_ptr<Identifier> {
	if (Type::name_v == name) { return std::make_unique<Type>(); }
	return {};
}
} // namespace

auto Identifier::try_strip_spec(std::string_view& out_format) -> Result<std::string_view> {
	if (out_format.size() < 2 || out_format.front() != open_v) { return {}; }

	auto const close_idx = out_format.find(Identifier::close_v);
	if (close_idx == std::string_view::npos) {
		return to_error(Error::Type::SyntaxError, std::format("missing identifier close ({}): '{}'", Identifier::close_v, out_format));
	}
	if (close_idx == 1) { return to_error(Error::Type::SyntaxError, std::format("missing identifier name: '{}'", out_format)); }

	auto ret = out_format.substr(0, close_idx + 1);
	out_format = out_format.substr(close_idx + 1);
	return ret;
}

auto Identifier::parse_spec(std::string_view spec_text) -> Result<Spec> {
	if (spec_text.size() < 2 || spec_text.front() != open_v || spec_text.back() != close_v) {
		return to_error(Error::Type::SyntaxError, std::format("not an identifier: '{}'", spec_text));
	}

	spec_text.remove_prefix(1);
	spec_text.remove_suffix(1);

	if (spec_text.empty()) { return to_error(Error::Type::SyntaxError, "identifier missing name"); }

	auto ret = Spec{};
	auto const delim_idx = spec_text.find(delim_v);
	if (delim_idx == std::string_view::npos) {
		ret.name = spec_text;
		return ret;
	}

	ret.name = spec_text.substr(0, delim_idx);

	auto const length_text = spec_text.substr(delim_idx + 1);
	auto const i_length = util::to_int(length_text);
	if (i_length < 0) { return to_error(Error::Type::SyntaxError, std::format("invalid length: '{}'", length_text)); }

	ret.max_length = std::size_t(i_length);

	return ret;
}

auto Identifier::create(Spec const spec) -> std::unique_ptr<Identifier> {
	if (auto ret = create_builtin<Year>(spec.name)) { return ret; }
	if (auto ret = create_builtin<Title>(spec.name)) { return ret; }
	return std::make_unique<Identifier>(std::string{spec.name}, spec.max_length);
}

auto Identifier::parse_value(std::string_view& out_text) const -> std::optional<std::string> {
	auto const length = [&] {
		if (m_max_length == 0) { return out_text.length(); }
		return std::min(m_max_length, out_text.length());
	}();

	auto ret = out_text.substr(0, length);
	out_text.remove_prefix(length);

	return std::string{ret};
}
} // namespace vifo
