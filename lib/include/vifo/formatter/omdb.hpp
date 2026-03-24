#pragma once
#include "klib/ptr.hpp"
#include "vifo/environment.hpp"
#include "vifo/formatter/formatter.hpp"
#include "vifo/omdb.hpp"

namespace vifo::formatter {
class Omdb : public IFormatter {
  public:
	explicit Omdb(omdb::IService const& omdb_service) : m_omdb_service(&omdb_service) {}

	[[nodiscard]] auto get_search_title(fs::path const& path) const -> std::string;

	std::string search_title_override{};

  protected:
	klib::Ptr<omdb::IService const> m_omdb_service{};

	Environment m_environment{};
};
} // namespace vifo::formatter
