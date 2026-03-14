#pragma once
#include "klib/base_types.hpp"
#include "vifo/result.hpp"
#include <cstddef>
#include <memory>
#include <string>
#include <string_view>

namespace vifo::detail {
class Identifier : public klib::Polymorphic {
  public:
	static constexpr auto open_v{'{'};
	static constexpr auto close_v{'}'};
	static constexpr auto delim_v{':'};

	struct Input;

	[[nodiscard]] static auto try_strip_input(std::string_view& out_text) -> Result<std::string_view>;
	[[nodiscard]] static auto parse_input(std::string_view text) -> Result<Input>;
	[[nodiscard]] static auto create(Input input) -> std::unique_ptr<Identifier>;

	explicit Identifier(std::string name, std::size_t max_length) : m_name(std::move(name)), m_max_length(max_length) {}

	[[nodiscard]] virtual auto parse_value(std::string_view& out_text) -> bool;

	[[nodiscard]] auto get_name() const -> std::string_view { return m_name; }
	[[nodiscard]] auto get_value() const -> std::string_view { return m_value; }

  protected:
	std::string m_value{};

  private:
	std::string m_name{};
	std::size_t m_max_length{};
};

class Year : public Identifier {
  public:
	static constexpr std::string_view name_v{"year"};

	explicit Year() : Identifier(std::string{name_v}, 4) {}

  private:
	[[nodiscard]] auto parse_value(std::string_view& out_text) -> bool final;
};

class Title : public Identifier {
  public:
	static constexpr std::string_view name_v{"title"};

	explicit Title() : Identifier(std::string{name_v}, 4) {}

  private:
	[[nodiscard]] auto parse_value(std::string_view& out_text) -> bool final;
};

struct Identifier::Input {
	std::string_view name{};
	std::size_t max_length{};
};
} // namespace vifo::detail
