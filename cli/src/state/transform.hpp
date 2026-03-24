#pragma once
#include "vifo/machine_state.hpp"
#include "vifo/manifest.hpp"
#include "vifo/util/progress.hpp"
#include <memory>

namespace vifo::cli {
class StateTransform : public MachineState, Manifest::Transformer {
  public:
	explicit StateTransform(Manifest manifest) : MachineState("Transform"), m_manifest(std::move(manifest)) {}

  private:
	auto execute() -> std::unique_ptr<MachineState> final;
	void on_transformed(Record const& record, Outcome outcome) const final;

	[[nodiscard]] auto confirm_rename() -> bool;

	Manifest m_manifest{};

	bool m_overwrite{};
	Transaction m_transaction{};
	std::unique_ptr<util::Progress> m_progress{};
};
} // namespace vifo::cli
