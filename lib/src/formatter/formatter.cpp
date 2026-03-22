#include "vifo/formatter/formatter.hpp"
#include "vifo/expression.hpp"

namespace vifo {
auto Formatter::interpolate(expression::Expression const& expression) const -> std::string {
	auto const get_value = [this](expression::Identifier const& identifier) { return m_environment->get_value(identifier.name); };
	return expression.interpolate(get_value);
}
} // namespace vifo
