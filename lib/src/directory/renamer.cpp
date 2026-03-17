#include "vifo/directory/renamer.hpp"
#include "vifo/directory/visitor.hpp"
#include "vifo/formatter.hpp"
#include "vifo/util/util.hpp"
#include <filesystem>

namespace vifo::directory {
namespace {
class VisitorImpl : public Visitor {
  public:
	explicit VisitorImpl(IFormatter& formatter) : m_formatter(formatter) {}

	Manifest manifest{};

	void visit(fs::path const& path) final {
		manifest.root = path;
		Visitor::visit(path);
	}

  private:
	auto accept_directory(fs::path const& path) -> bool final {
		auto const src_filename = path.filename().generic_string();
		if (auto const dst_filename = m_formatter.format(src_filename); !dst_filename.empty()) {
			auto dst_path = util::prefix_parent(path, dst_filename);
			auto const exists = fs::exists(dst_path);
			manifest.entries.push_back(Manifest::Entry{.source = path, .destination = std::move(dst_path), .exists = exists});
			if (exists) { ++manifest.collision_count; }
		}
		return true;
	}

	IFormatter& m_formatter;
};
} // namespace

auto Renamer::create_interpolator(std::string input_format, std::string output_format) -> Result<Renamer> {
	return vifo::create_interpolator(std::move(input_format), std::move(output_format)).transform([](std::unique_ptr<IFormatter> interpolator) {
		return Renamer{std::move(interpolator)};
	});
}

auto Renamer::build_manifest(fs::path const& root) const -> Manifest {
	if (!m_formatter || !fs::is_directory(root)) { return {}; }

	auto visitor = VisitorImpl{*m_formatter};
	visitor.visit(root);
	return std::move(visitor.manifest);
}
} // namespace vifo::directory
