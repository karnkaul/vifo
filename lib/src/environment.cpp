#include "vifo/environment.hpp"
#include <algorithm>

namespace vifo {
auto Environment::find_symbol(std::string_view const name) const -> klib::Ptr<Symbol> {
	auto const it = std::ranges::find_if(symbols, [name](auto const& b) { return b->get_name() == name; });
	if (it == symbols.end()) { return nullptr; }
	return it->get();
}

auto Environment::get_value(std::string_view const name) const -> std::string_view {
	if (auto const symbol = find_symbol(name)) { return symbol->value; }
	return {};
}
} // namespace vifo
