#pragma once
#include "klib/base_types.hpp"
#include "vifo/result.hpp"
#include <cstddef>
#include <memory>
#include <optional>
#include <string>
#include <string_view>

namespace vifo {
class Identifier : public klib::Polymorphic {
  public:
	static constexpr auto open_v{'{'};
	static constexpr auto close_v{'}'};
	static constexpr auto delim_v{':'};

	struct Spec;

	[[nodiscard]] static auto try_strip_spec(std::string_view& out_format) -> ResultOld<std::string_view>;
	[[nodiscard]] static auto parse_spec(std::string_view spec_text) -> ResultOld<Spec>;
	[[nodiscard]] static auto create(Spec spec) -> std::unique_ptr<Identifier>;

	explicit Identifier(std::string name, std::size_t max_length) : m_name(std::move(name)), m_max_length(max_length) {}

	[[nodiscard]] virtual auto parse_value(std::string_view& out_text) const -> std::optional<std::string>;

	[[nodiscard]] auto get_name() const -> std::string_view { return m_name; }

  private:
	std::string m_name{};
	std::size_t m_max_length{};
};

struct Identifier::Spec {
	std::string_view name{};
	std::size_t max_length{};
};
} // namespace vifo
