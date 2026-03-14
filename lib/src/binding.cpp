#include "vifo/binding.hpp"
#include "detail/identifier.hpp"
#include "klib/assert.hpp"
#include <string_view>

namespace vifo {
namespace detail {
namespace {
[[nodiscard]] auto to_binding(std::string_view& out_input, Identifier& identifier) -> Binding {
	if (!identifier.parse_value(out_input)) { return {}; }
	return Binding{.key = std::string{identifier.get_name()}, .value = std::string{identifier.get_value()}};
}

class Extractor {
  public:
	explicit Extractor(std::string_view const format, std::string_view const input) : m_format(format), m_input(input) {}

	[[nodiscard]] auto operator()() -> Result<std::vector<Binding>> {
		while (!m_format.empty() && !m_input.empty()) {
			auto binding = try_parse_binding();
			if (!binding) { return std::unexpected{binding.error()}; }

			if (!binding->key.empty()) {
				m_bindings.push_back(std::move(*binding));
				continue;
			}

			if (!advance_if_match()) { return {}; }
		}

		if (!m_format.empty()) { return {}; }

		return std::move(m_bindings);
	}

  private:
	[[nodiscard]] auto try_parse_binding() -> Result<Binding> {
		return Identifier::try_strip_input(m_format).and_then([&](std::string_view const input_text) { return parse_identifier(input_text); });
	}

	[[nodiscard]] auto parse_identifier(std::string_view const input_text) -> Result<Binding> {
		if (input_text.empty()) { return {}; }
		return Identifier::parse_input(input_text).transform([&](Identifier::Input const input) {
			auto const identifier = Identifier::create(input);
			KLIB_ASSERT(identifier);
			return to_binding(m_input, *identifier);
		});
	}

	[[nodiscard]] auto advance_if_match() -> bool {
		KLIB_ASSERT(!m_format.empty());
		if (m_input.empty() || m_input.front() != m_format.front()) { return false; }
		m_input.remove_prefix(1);
		m_format.remove_prefix(1);
		return true;
	}

	std::string_view m_format{};
	std::string_view m_input{};

	std::vector<Binding> m_bindings{};
};
} // namespace
} // namespace detail

auto Binding::extract(std::string_view format, std::string_view input) -> Result<std::vector<Binding>> { return detail::Extractor{format, input}(); }
} // namespace vifo
