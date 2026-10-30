#pragma once
#include "vifo/expression.hpp"
#include "vifo/manifest.hpp"
#include "vifo/media_file.hpp"
#include "vifo/transaction.hpp"
#include "vifo/types.hpp"

namespace vifo::detail {
[[nodiscard]] auto to_error(Error::Type type, expression::Token token, std::string_view input, std::string_view msg) -> std::unexpected<Error>;
[[nodiscard]] auto to_error(Error::Type type, std::string_view msg) -> std::unexpected<Error>;

[[nodiscard]] inline auto to_record(fs::path source, fs::path destination, Operation const operation) -> Record {
	return Record{
		.source = std::move(source),
		.destination = std::move(destination),
		.operation = operation,
	};
}

[[nodiscard]] auto if_directory(fs::path const& path) -> Result<fs::path>;

[[nodiscard]] auto cleanup_filestem(fs::path const& path) -> std::string;

void filter_en_subtitles(Manifest& out_manifest, std::vector<MediaFile>& out_files, fs::path const& video);
void ensure_relative_paths(Manifest& out_manifest);
} // namespace vifo::detail
