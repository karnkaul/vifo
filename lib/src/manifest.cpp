#include "vifo/manifest.hpp"
#include "vifo/formatter.hpp"
#include "vifo/util/util.hpp"
#include <filesystem>

namespace vifo {
namespace {
class Scanner {
  public:
	explicit Scanner(IFormatter& formatter, Manifest& out) : m_formatter(formatter), m_manifest(out) {}

	void build_manifest(fs::path const& root) {
		m_root = root;
		if (fs::is_regular_file(m_root)) {
			accept_path(m_root);
			return;
		}

		if (!fs::is_directory(m_root)) { return; }
		iterate(m_root);
	}

  private:
	void iterate(fs::path const& directory) {
		for (auto const& it : fs::directory_iterator{directory}) {
			if (it.is_directory()) {
				iterate(it.path());
				accept_path(it.path());
				continue;
			}

			if (it.is_regular_file()) { accept_path(it.path()); }
		}
	}

	void accept_path(fs::path const& path) {
		auto const src_filename = path.filename().generic_string();
		if (auto const dst_filename = m_formatter.format(src_filename); !dst_filename.empty()) {
			auto dst_path = util::prefix_parent(path, dst_filename);
			if (fs::exists(dst_path)) { ++m_manifest.collision_count; }
			dst_path = fs::relative(dst_path, m_root);
			m_manifest.entries.push_back(Manifest::Entry{.source = fs::relative(path, m_root), .destination = std::move(dst_path)});
		}
	}

	IFormatter& m_formatter;
	Manifest& m_manifest;

	fs::path m_root{};
};
} // namespace

auto Manifest::build(IFormatter& formatter, fs::path const& root) -> Manifest {
	auto ret = Manifest{};
	append_to(ret, formatter, root);
	return ret;
}

void Manifest::append_to(Manifest& out, IFormatter& formatter, fs::path const& subdirectory) {
	auto const path = util::concat_path(out.root, subdirectory);
	if (!fs::is_directory(path)) { return; }

	if (out.root.empty()) { out.root = path; }
	Scanner{formatter, out}.build_manifest(path);
}
} // namespace vifo
