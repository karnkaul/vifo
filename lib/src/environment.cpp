#include "vifo/environment.hpp"
#include "klib/visitor.hpp"
#include "vifo/util/util.hpp"
#include <algorithm>

namespace vifo {
auto Environment::find_symbol(std::string_view const name) const -> klib::Ptr<Symbol const> {
	auto const it = std::ranges::find_if(symbols, [name](Symbol const& b) { return b.get_name() == name; });
	if (it == symbols.end()) { return nullptr; }
	return &*it;
}

auto Environment::find_symbol(std::string_view name) -> klib::Ptr<Symbol> {
	// NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
	return const_cast<Symbol*>(std::as_const(*this).find_symbol(name).get());
}

auto Environment::get_value(std::string_view const name) const -> std::string_view {
	if (auto const symbol = find_symbol(name)) { return symbol->value; }
	return {};
}

void Environment::set_symbol(std::string_view const name, std::string value) {
	auto symbol = find_symbol(name);
	if (!symbol) {
		symbols.emplace_back(std::string{name});
		symbol = &symbols.back();
	}
	symbol->value = std::move(value);
}

auto Environment::interpolate(expression::Expression const& expression) const -> std::string {
	auto ret = std::string{};
	auto const visitor = klib::Visitor{
		[&](expression::Substring const& substring) { ret += substring.text; },
		[&](expression::Identifier const& identifier) { ret += get_value(identifier.name); },
	};
	for (auto const& atom : expression.atoms) { std::visit(visitor, atom.value); }
	return util::sanitize_for_path(ret);
}
} // namespace vifo
