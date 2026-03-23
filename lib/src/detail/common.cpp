#include "detail/common.hpp"
#include "vifo/media/file.hpp"
#include "vifo/util/word_scanner.hpp"
#include <format>
#include <string_view>

namespace vifo {
namespace {
[[nodiscard]] auto format_message(Error::Type const type, std::string_view const msg) {
	return std::format("[{}] {}", Error::type_name_map.to_name(type), msg);
}

[[nodiscard]] auto is_en_subtitle(fs::path const& path) {
	auto stem = path.stem().string();
	for (char& c : stem) { c = char(std::tolower(static_cast<unsigned char>(c))); }

	auto scanner = util::WordScanner{stem};
	auto word = util::WordToken{};
	while (scanner.next(word)) {
		if (word.lexeme == "en" || word.lexeme == "eng" || word.lexeme == "english") { return true; }
	}
	return false;
}
} // namespace

auto detail::to_error(Error::Type type, expression::Token token, std::string_view input, std::string_view msg) -> std::unexpected<Error> {
	auto ret = Error{.type = type, .message = format_message(type, msg)};
	if (!input.empty()) {
		std::format_to(std::back_inserter(ret.message), "\n | {}\n | ", input);
		for (std::size_t i = 0; i < token.start_index; ++i) { ret.message.push_back(' '); }
		for (std::size_t i = 0; i < token.length; ++i) { ret.message.push_back('^'); }
	}
	return std::unexpected{std::move(ret)};
}

auto detail::to_error(Error::Type type, std::string_view msg) -> std::unexpected<Error> {
	return std::unexpected{Error{.type = type, .message = format_message(type, msg)}};
}

void detail::filter_en_subtitles(Manifest& out_manifest, std::vector<MediaFile>& out_files) {
	auto const transfer = [&](MediaFile& file) {
		if (!is_en_subtitle(file.path)) {
			out_manifest.orphans.push_back(Manifest::Entry{.source = std::move(file.path), .type = file.type});
			return true;
		}
		return false;
	};
	std::erase_if(out_files, transfer);
}
} // namespace vifo
