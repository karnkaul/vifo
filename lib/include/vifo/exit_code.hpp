#pragma once
#include <cstdint>
#include <cstdlib>

namespace vifo {
enum class ExitCode : std::int8_t {
	Success = EXIT_SUCCESS,
	Failure = EXIT_FAILURE,

	SyntaxError = 10,
	FormatError = 11,

	InvalidArgument = 10,
	SourceFailure = 11,
	ManifestFailure = 12,
	TransformFailure = 13,
	OmdbServiceFailure = 15,
};
} // namespace vifo
