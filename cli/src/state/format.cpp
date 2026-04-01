#include "state/format.hpp"
#include "vifo/formatter/formatter.hpp"
#include "vifo/util/progress.hpp"
#include <print>

namespace vifo::cli {
namespace {
void print_transaction(Transaction const& transaction) {
	if (!transaction.failure.empty()) {
		std::println(stderr, "[!] some transforms failed:");
		auto table = Transaction::format_destinations_table(transaction.parent, transaction.failure);
		std::println(stderr, "{}", table);
	}
	if (!transaction.pass.empty()) {
		std::println("pass:");
		auto table = Transaction::format_destinations_table(transaction.parent, transaction.pass);
		std::println(stderr, "{}", table);
	}
	if (!transaction.success.empty()) {
		std::println("success:");
		auto table = Transaction::format_destinations_table(transaction.parent, transaction.success);
		std::println(stderr, "{}", table);
	}
}
} // namespace

StateFormat::StateFormat(IFormatter& formatter, fs::path directory, std::unique_ptr<MachineState> next)
	: MachineState("Transform"), m_formatter(&formatter), m_directory(std::move(directory)), m_next(std::move(next)) {}

auto StateFormat::execute() -> std::unique_ptr<MachineState> {
	auto manifest = m_formatter->generate_manifest(m_directory);
	if (!manifest) {
		// TODO: override title
		return handle_error(manifest.error());
	}

	if (manifest->entries.empty()) {
		std::println("nothing to rename");
		return {};
	}

	std::println("parent: {}", manifest->parent.generic_string());
	if (!manifest->orphans.empty()) { std::println("orphans:\n{}", manifest->format_orphans_table()); }
	std::println("{}", manifest->format_entries_tables());

	auto const metrics = manifest->compute_metrics();
	if (metrics.duplicates > 0) {
		return set_error(ExitCode::DuplicateDestinations, std::format("{} duplicate destinations! aborting", metrics.duplicates + 1));
	}

	std::println("{} entries to rename, {} existing", manifest->entries.size(), metrics.existing);

	if (!confirm_rename(*manifest)) { return {}; }

	m_progress = std::make_unique<util::Progress>(std::int64_t(manifest->entries.size()));
	auto transaction = transform_manifest(*manifest, Operation::Rename, m_overwrite);
	m_progress->finish();
	print_transaction(transaction);

	if (transaction.success.empty()) { return set_error(ExitCode::TransformFailure); }

	if (!should_continue("rollback?")) {
		if (!transaction.failure.empty()) { m_exit_code = ExitCode::TransformFailure; }
		return {};
	}

	transaction = transaction.rollback();
	if (!transaction.failure.empty()) { m_exit_code = ExitCode::TransformFailure; }
	print_transaction(transaction);

	std::println("transform complete");

	return std::move(m_next);
}

void StateFormat::on_transformed(Record const& /*record*/, Outcome const /*outcome*/) const { m_progress->increment_completed(); }

auto StateFormat::confirm_rename(Manifest const& manifest) -> bool {
	auto const metrics = manifest.compute_metrics();
	if (metrics.existing == 0) { return should_continue("rename?"); }

	std::println("rename:");
	auto const options = std::array{
		prompt::Option{.text = "overwrite existing", .callback = [this] { m_overwrite = true; }},
		prompt::Option{.text = "skip existing", .callback = [this] { m_overwrite = false; }},
	};
	return should_continue(options);
}
} // namespace vifo::cli
