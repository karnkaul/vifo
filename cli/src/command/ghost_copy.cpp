#include "command/ghost_copy.hpp"
#include "vifo/machine_state.hpp"
#include "vifo/path/list.hpp"
#include "vifo/path/scanner.hpp"
#include "vifo/types.hpp"
#include "vifo/util/util.hpp"
#include <algorithm>
#include <filesystem>
#include <print>
#include <string_view>

namespace vifo::cli::command {
namespace fs = std::filesystem;

namespace {
struct Storage {
	std::string_view source{};
	std::string_view destination{};
	bool directories_only{};
	bool overwrite{};
	int max_depth{10};

	path::List list{};
};

class Scanner : private path::ListScanner {
  public:
	explicit Scanner(std::string_view const source, bool const directories_only) : m_source(source), m_directories_only(directories_only) {}

	[[nodiscard]] auto scan(int const max_depth) -> path::List { return path::ListScanner::scan_paths(m_source, max_depth); }

  private:
	[[nodiscard]] auto should_store(fs::path const& path) const -> bool final {
		if (path == m_source) { return false; }
		if (m_directories_only) { return fs::is_directory(path); }
		return true;
	}

	std::string_view m_source{};
	bool m_directories_only{};
};

class StateScanSources : public MachineState {
  public:
	explicit StateScanSources(Storage storage) : MachineState("ScanSources"), m_storage(std::move(storage)), m_max_depth(storage.max_depth) {}

  private:
	auto execute() -> std::unique_ptr<MachineState> final {
		auto scanner = Scanner{m_storage.source, m_storage.directories_only};
		m_storage.list = scanner.scan(m_max_depth);
		if (m_storage.list.paths.empty()) {
			std::println("nothing to copy");
			return {};
		}
		std::ranges::reverse(m_storage.list.paths);

		std::println("{}\ndestination: {}", m_storage.list.format_table(), m_storage.destination);
		if (!should_continue()) { return {}; }

		auto const result = util::ghost_copy(m_storage.source, m_storage.destination, m_storage.overwrite);
		if (result < 0) { return set_error(ExitCode::Failure, "incomplete / error"); }

		std::println("{} entries replicated", result);
		return {};
	}

	Storage m_storage{};
	int m_max_depth{};
};
} // namespace

auto GhostCopy::get_parameters() -> std::vector<clap::Parameter> {
	return {
		clap::named_flag(m_directories_only, "d,dir-only", "only mirror directories"),
		clap::named_flag(m_overwrite, "o,overwrite", "overwrite existing entries"),
		clap::named_option(m_max_depth, "m,max-depth", "max iteration depth"),
		clap::positional_required(m_source, "SRC", "source directory to mirror"),
		clap::positional_optional(m_destination, "DST", "mirror destination (default = .)"),
	};
}

auto GhostCopy::execute() -> ExitCode {
	if (!fs::is_directory(m_source)) {
		std::println(stderr, "invalid source directory: '{}'", m_source);
		return ExitCode::InvalidArgument;
	}

	auto storage = Storage{
		.source = m_source,
		.destination = m_destination,
		.directories_only = m_directories_only,
		.overwrite = m_overwrite,
		.max_depth = m_max_depth,
	};
	return execute_state_machine(std::make_unique<StateScanSources>(std::move(storage)));
}
} // namespace vifo::cli::command
