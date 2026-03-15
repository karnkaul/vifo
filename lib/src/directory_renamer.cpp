#include "vifo/directory_renamer.hpp"
#include "vifo/directory_visitor.hpp"
#include "vifo/formatter.hpp"
#include "vifo/util.hpp"
#include <filesystem>

namespace vifo {
namespace {
class Visitor : public DirectoryVisitor {
  public:
	explicit Visitor(IFormatter& formatter) : m_formatter(formatter) {}

	Manifest manifest{};

	void visit(fs::path const& path) final {
		manifest.root = path;
		DirectoryVisitor::visit(path);
	}

  private:
	auto accept_directory(fs::path const& path) -> bool final {
		auto const src_filename = path.filename().generic_string();
		if (auto const dst_filename = m_formatter.format(src_filename); !dst_filename.empty()) {
			auto dst_path = util::prefix_parent(path, dst_filename);
			manifest.entries.push_back(Manifest::Entry{.source = path, .destination = std::move(dst_path)});
		}
		return true;
	}

	IFormatter& m_formatter;
};
} // namespace

auto DirectoryRenamer::create_interpolator(std::string_view const input_expression, std::string_view const output_expression) -> Result<DirectoryRenamer> {
	return vifo::create_interpolator(input_expression, output_expression).transform([](std::unique_ptr<IFormatter> interpolator) {
		return DirectoryRenamer{std::move(interpolator)};
	});
}

auto DirectoryRenamer::build_manifest(fs::path const& root) const -> Manifest {
	if (!m_formatter || !fs::is_directory(root)) { return {}; }

	auto visitor = Visitor{*m_formatter};
	visitor.visit(root);
	return std::move(visitor.manifest);
}
} // namespace vifo
