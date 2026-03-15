#include "vifo/environment.hpp"
#include "klib/assert.hpp"
#include "vifo/identifier.hpp"
#include "vifo/result.hpp"
#include <expected>
#include <string>
#include <string_view>

namespace vifo {
namespace {
struct Binding {
	std::string key{};
	std::string value{};
};

[[nodiscard]] auto to_binding(std::string_view& out_input, Identifier const& identifier) -> Binding {
	auto value = identifier.parse_value(out_input);
	if (!value) { return {}; }
	return Binding{.key = std::string{identifier.get_name()}, .value = std::move(*value)};
}

auto parse_identifier(std::string_view const id_text) -> ResultOld<std::unique_ptr<Identifier>> {
	if (id_text.empty()) { return {}; }
	return Identifier::parse_spec(id_text).transform([&](Identifier::Spec const spec) { return Identifier::create(spec); });
}

class Extractor {
  public:
	explicit Extractor(std::span<std::unique_ptr<Identifier const> const> const& identifiers, std::string_view const format, std::string_view const text)
		: m_identifiers(identifiers), m_format(format), m_input(text) {}

	auto operator()() -> ResultOld<std::vector<Binding>> {
		while (!m_format.empty() && !m_input.empty()) {
			auto id_text = Identifier::try_strip_spec(m_format);
			if (!id_text) { return std::unexpected{std::move(id_text.error())}; }

			if (!id_text->empty()) {
				auto spec = Identifier::parse_spec(*id_text);
				if (!spec) { return std::unexpected{std::move(spec.error())}; }
				auto const it = std::ranges::find_if(m_identifiers, [name = spec->name](auto const& id) { return id->get_name() == name; });
				if (it == m_identifiers.end()) { return {}; }

				m_bindings.push_back(to_binding(m_input, **it));
				continue;
			}

			if (!advance_if_match()) { return {}; }
		}

		if (!m_format.empty()) { return {}; }

		return std::move(m_bindings);
	}

  private:
	[[nodiscard]] auto advance_if_match() -> bool {
		KLIB_ASSERT(!m_format.empty());
		if (m_input.empty() || m_input.front() != m_format.front()) { return false; }
		m_input.remove_prefix(1);
		m_format.remove_prefix(1);
		return true;
	}

	std::span<std::unique_ptr<Identifier const> const> m_identifiers;
	std::string_view m_format{};
	std::string_view m_input{};

	std::vector<Binding> m_bindings{};
};
} // namespace

auto Environment::create(std::string_view input_format) -> ResultOld<Environment> {
	auto ret = Environment{};
	ret.m_input_format = input_format;

	while (!input_format.empty()) {
		auto result = Identifier::try_strip_spec(input_format).and_then(&parse_identifier);
		if (!result) { return std::unexpected{std::move(result.error())}; }

		auto identifier = std::move(*result);
		if (!identifier) {
			input_format.remove_prefix(1);
			continue;
		}

		ret.m_identifiers.push_back(std::move(identifier));
	}

	return ret;
}

auto Environment::interpolate(std::string_view const input, std::string_view const output_format) const -> ResultOld<std::string> {
	auto const bindings = Extractor{m_identifiers, m_input_format, input}();
	if (!bindings) { return std::unexpected{std::move(bindings.error())}; }

	auto ret = std::string{};
	for (auto expression = output_format; !expression.empty();) {
		auto id_text = Identifier::try_strip_spec(expression);
		if (!id_text) { return std::unexpected{std::move(id_text.error())}; }

		if (id_text->empty()) {
			ret.push_back(expression.front());
			expression.remove_prefix(1);
			continue;
		}

		auto spec = Identifier::parse_spec(*id_text);
		if (!spec) { return std::unexpected{std::move(spec.error())}; }
		auto const it = std::ranges::find_if(*bindings, [name = spec->name](Binding const& b) { return b.key == name; });
		if (it == bindings->end()) { return {}; }

		ret += it->value;
	}

	return ret;
}
} // namespace vifo
