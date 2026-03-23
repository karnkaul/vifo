#pragma once
#include "klib/base_types.hpp"
#include "klib/ptr.hpp"
#include "vifo/expression.hpp"
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace vifo {
class Symbol : public klib::Polymorphic {
  public:
	explicit Symbol(std::string name) : m_name(std::move(name)) {}

	[[nodiscard]] auto get_name() const -> std::string_view { return m_name; }

	std::string value{};

  private:
	std::string m_name{};
};

struct Environment {
	[[nodiscard]] auto find_symbol(std::string_view name) const -> klib::Ptr<Symbol>;
	[[nodiscard]] auto get_value(std::string_view name) const -> std::string_view;

	void set_symbol(std::string_view name, std::string value);

	[[nodiscard]] auto interpolate(expression::Expression const& expression) const -> std::string;

	std::vector<std::unique_ptr<Symbol>> symbols{};
};
} // namespace vifo
