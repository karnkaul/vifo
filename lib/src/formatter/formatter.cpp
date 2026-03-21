#include "vifo/formatter/formatter.hpp"
#include "vifo/expression.hpp"
#include "vifo/util/util.hpp"
#include <filesystem>

namespace vifo {
auto Formatter::format_path(fs::path const& path) -> fs::path {
	auto const stem = format_string(path.stem().generic_string());
	if (stem.empty()) { return {}; }
	auto ret = util::prefix_parent(path, stem);
	ret += path.extension();
	return ret;
}

auto Formatter::interpolate(expression::Expression const& expression) const -> std::string {
	auto const get_value = [this](expression::Identifier const& identifier) { return m_environment->get_value(identifier.name); };
	return expression.interpolate(get_value);
}
} // namespace vifo
