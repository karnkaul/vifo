#include "vifo/path/list.hpp"
#include "vifo/util/util.hpp"
#include <array>
#include <filesystem>

namespace vifo::path {
auto List::format_table() const -> std::string {
	static constexpr auto headers_v = std::array{"name"};
	auto const per_record = [this](std::vector<std::string>& row, fs::path const& path) { row.push_back(util::to_relative(scan_path, path).generic_string()); };
	return util::format_enumerated_table(headers_v, paths, per_record);
}
} // namespace vifo::path
