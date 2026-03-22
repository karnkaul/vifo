#pragma once
#include "klib/base_types.hpp"
#include "vifo/transaction.hpp"
#include "vifo/types.hpp"
#include <cstdint>
#include <vector>

namespace vifo {
namespace fs = std::filesystem;

struct Manifest {
	struct Metrics {
		std::int64_t existing{};
		std::int64_t duplicates{};
	};

	struct Entry {
		fs::path source{};
		fs::path destination{};
	};

	class Transformer;

	[[nodiscard]] auto format_table() const -> std::string;

	fs::path parent{};
	std::vector<Entry> entries{};
	Metrics metrics{};
};

class Manifest::Transformer : public klib::Polymorphic {
  public:
	[[nodiscard]] auto transform_manifest(Manifest const& manifest, Operation operation, bool overwrite) const -> Transaction;

  protected:
	virtual void on_transformed([[maybe_unused]] Record const& record, [[maybe_unused]] Outcome outcome) const {}
};
} // namespace vifo
