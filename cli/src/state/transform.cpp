#include "state/transform.hpp"
#include <print>

namespace vifo::cli {
namespace {
void print_transaction(Transaction const& transaction) {
	if (!transaction.failure.empty()) {
		std::println(stderr, "[!] some transforms failed:");
		auto table = Transaction::format_table(transaction.parent, transaction.failure);
		std::println(stderr, "{}", table);
	}
	if (!transaction.pass.empty()) {
		std::println("pass:");
		auto table = Transaction::format_table(transaction.parent, transaction.pass);
		std::println(stderr, "{}", table);
	}
	if (!transaction.success.empty()) {
		std::println("success:");
		auto table = Transaction::format_table(transaction.parent, transaction.success);
		std::println(stderr, "{}", table);
	}
}
} // namespace

auto StateTransform::execute() -> std::unique_ptr<MachineState> {
	if (!confirm_rename()) { return {}; }

	m_progress = std::make_unique<util::Progress>(std::int64_t(m_manifest.entries.size()));
	m_transaction = transform_manifest(m_manifest, Operation::Rename, m_overwrite);
	m_progress->finish();
	print_transaction(m_transaction);

	if (m_transaction.success.empty()) { return set_error(ExitCode::TransformFailure); }

	if (!should_continue("rollback?")) {
		if (!m_transaction.failure.empty()) { m_exit_code = ExitCode::TransformFailure; }
		return {};
	}

	m_transaction = m_transaction.rollback();
	if (!m_transaction.failure.empty()) { m_exit_code = ExitCode::TransformFailure; }
	print_transaction(m_transaction);

	std::println("transform complete");

	return {};
}

void StateTransform::on_transformed(Record const& /*record*/, Outcome const /*outcome*/) const { m_progress->increment_completed(); }

auto StateTransform::confirm_rename() -> bool {
	auto const metrics = m_manifest.compute_metrics();
	if (metrics.existing == 0) { return should_continue("rename?"); }

	std::println("rename:");
	auto const options = std::array{
		prompt::Option{.text = "overwrite existing", .callback = [this] { m_overwrite = true; }},
		prompt::Option{.text = "skip existing", .callback = [this] { m_overwrite = false; }},
	};
	return should_continue(options);
}
} // namespace vifo::cli
