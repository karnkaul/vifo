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

void Environment::set_symbol(std::string_view const name, std::string value) {
	auto symbol = find_symbol(name);
	if (!symbol) {
		symbols.push_back(std::make_unique<Symbol>(std::string{name}));
		symbol = symbols.back().get();
	}
	symbol->value = std::move(value);
}
} // namespace vifo
