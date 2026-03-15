#include "vifo/interpolator.hpp"
#include "vifo/expression.hpp"
#include "vifo/result.hpp"
#include "vifo/util.hpp"
#include <string_view>
#include <unordered_map>

namespace vifo {
namespace {
class IReference : public klib::Polymorphic {
  public:
	explicit IReference(std::string name) : m_name(std::move(name)) {}

	virtual auto parse_value(std::string_view& out_input, std::string& out_output) const -> bool = 0;

	[[nodiscard]] auto get_name() const -> std::string_view { return m_name; }

  private:
	std::string m_name{};
};

class Variable : public IReference {
  public:
	explicit Variable(std::string name, std::size_t const max_length) : IReference(std::move(name)), m_max_length(max_length) {}

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

class Year : public IReference {
  public:
	static constexpr std::string_view name_v{"year"};

	explicit Year() : IReference(std::string{name_v}) {}

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

[[nodiscard]] auto create_reference(expression::Identifier const& identifier, bool& is_zero_length_variable) -> std::unique_ptr<IReference> {
	if (identifier.name == "year") { return std::make_unique<Year>(); }
	is_zero_length_variable = identifier.length == 0;
	return std::make_unique<Variable>(std::string{identifier.name}, identifier.length);
}

using Symbol = std::variant<expression::Substring, std::unique_ptr<IReference const>>;

struct Source {
	std::vector<Symbol> symbols{};
};

struct Context {
	Source source{};
	expression::Expression transform{};
	std::unordered_map<std::string_view, std::string> value_map{};
};

class Interpolator : public IInterpolator {
  public:
	[[nodiscard]] auto initialize(Expression input, Expression output) -> Result<void> {
		return build_source(std::move(input)).and_then([&] { return build_transform(std::move(output)); });
	}

  private:
	[[nodiscard]] auto format(std::string_view const input, std::string& output) -> bool final {
		if (!extract_values(input)) { return false; }
		return interpolate(output);
	}

	[[nodiscard]] auto build_source(Expression input) -> Result<void> {
		auto zero_length_id = false;
		for (auto& atom : input.atoms) {
			if (auto* substring = std::get_if<expression::Substring>(&atom)) {
				m_context.source.symbols.emplace_back(std::move(*substring));
				continue;
			}

			auto is_zero_length_variable = false;
			auto reference = create_reference(std::get<expression::Identifier>(atom), is_zero_length_variable);
			if (is_zero_length_variable) {
				if (zero_length_id) {
					return to_error(Error::Type::Format,
									std::format("excess identifier after 0-length parsed: '{}'", std::get<expression::Identifier>(atom).name));
				}
				zero_length_id = true;
			}

			auto const name = reference->get_name();
			if (m_context.value_map.contains(name)) {
				return to_error(Error::Type::Format, std::format("duplicate identifier in input expression: '{}'", name));
			}

			m_context.value_map.insert_or_assign(name, std::string{});
			m_context.source.symbols.emplace_back(std::move(reference));
		}

		return {};
	}

	[[nodiscard]] auto build_transform(Expression transform) -> Result<void> {
		for (auto const& atom : transform.atoms) {
			auto const* identifier = std::get_if<expression::Identifier>(&atom);
			if (!identifier) { continue; }

			if (!m_context.value_map.contains(identifier->name)) {
				return to_error(Error::Type::Format, std::format("undefined identifier in output expression: '{}'", identifier->name));
			}
		}

		m_context.transform = std::move(transform);
		return {};
	}

	[[nodiscard]] auto extract_values(std::string_view input) -> bool {
		for (auto const& symbol : m_context.source.symbols) {
			if (!match_symbol(input, symbol)) { return false; }
		}
		return true;
	}

	[[nodiscard]] auto match_symbol(std::string_view& out_input, Symbol const& symbol) -> bool {
		if (auto const* substring = std::get_if<expression::Substring>(&symbol)) { return substring->consume(out_input); }

		auto const& reference = std::get<std::unique_ptr<IReference const>>(symbol);
		auto& value = m_context.value_map[reference->get_name()];
		return reference->parse_value(out_input, value);
	}

	[[nodiscard]] auto interpolate(std::string& out) const -> bool {
		for (auto const& atom : m_context.transform.atoms) {
			if (auto const* substring = std::get_if<expression::Substring>(&atom)) {
				out += substring->text;
				continue;
			}

			auto const& identifer = std::get<expression::Identifier>(atom);
			auto const it = m_context.value_map.find(identifer.name);
			if (it == m_context.value_map.end()) { return false; }
			out += it->second;
		}

		return true;
	}

	Context m_context{};
};
} // namespace

auto IInterpolator::create(Expression input, Expression output) -> Result<std::unique_ptr<IInterpolator>> {
	auto ret = std::make_unique<Interpolator>();
	return ret->initialize(std::move(input), std::move(output)).transform([&] { return std::move(ret); });
}
} // namespace vifo
