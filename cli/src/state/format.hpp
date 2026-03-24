#pragma once
#include "klib/ptr.hpp"
#include "vifo/formatter/formatter.hpp"
#include "vifo/machine_state.hpp"
#include "vifo/manifest.hpp"
#include "vifo/util/progress.hpp"
#include <filesystem>
#include <memory>

namespace vifo::cli {
class StateFormat : public MachineState, Manifest::Transformer {
  public:
	explicit StateFormat(IFormatter& formatter, fs::path directory, std::unique_ptr<MachineState> next = {});

  private:
	auto execute() -> std::unique_ptr<MachineState> final;
	void on_transformed(Record const& record, Outcome outcome) const final;

	[[nodiscard]] auto confirm_rename(Manifest const& manifest) -> bool;

	klib::Ptr<IFormatter> m_formatter{};
	fs::path m_directory{};
	std::unique_ptr<MachineState> m_next{};

	bool m_overwrite{};
	std::unique_ptr<util::Progress> m_progress{};
};
} // namespace vifo::cli
