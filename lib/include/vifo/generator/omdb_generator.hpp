#pragma once
#include "klib/ptr.hpp"
#include "vifo/environment.hpp"
#include "vifo/generator/generator.hpp"
#include "vifo/omdb.hpp"

namespace vifo {
class IOmdbGenerator : public IGenerator {
  public:
	explicit IOmdbGenerator(omdb::IService const& omdb_service) : m_omdb_service(&omdb_service) {}

  protected:
	klib::Ptr<omdb::IService const> m_omdb_service{};

	Environment m_environment{};
};
} // namespace vifo
