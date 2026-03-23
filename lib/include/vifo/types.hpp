#pragma once
#include "klib/enum/name.hpp"
#include <cstdint>
#include <string>

namespace vifo {
struct Error {
	enum class Type : std::int8_t {
		Argument,
		Syntax,
		Format,
		Identify,
		Http,
	};

	inline static auto const type_name_map = klib::EnumNameMap<Type>{
		{Type::Syntax, "ArgumentError"},   {Type::Syntax, "SyntaxError"}, {Type::Format, "FormatError"},
		{Type::Identify, "IdentifyError"}, {Type::Http, "HttpError"},
	};

	Type type{};
	std::string message{};
};

enum class ExitCode : std::int8_t {
	Success = EXIT_SUCCESS,
	Failure = EXIT_FAILURE,

	InvalidArgument = 10,
	SyntaxError = 11,
	FormatError = 12,
	IdentifyError = 13,
	HttpError = 14,

	IoError = 20,
	ForcedHalt = 21,

	DuplicateDestinations = 30,
	TransformFailure = 31,
};

enum class Operation : std::int8_t { Rename, Copy, Delete };
inline auto const operation_name_map = klib::EnumNameMap<Operation>{
	{Operation::Rename, "Rename"},
	{Operation::Copy, "Copy"},
	{Operation::Delete, "Delete"},
};

enum class Outcome : std::int8_t { Success, Failure, Pass };
inline auto const outcome_name_map = klib::EnumNameMap<Outcome>{
	{Outcome::Success, "Success"},
	{Outcome::Failure, "Failure"},
	{Outcome::Pass, "Pass"},
};

enum class MediaFileType : std::int8_t { Video, Subtitle };
inline auto const media_file_type_name_map = klib::EnumNameMap<MediaFileType>{
	{MediaFileType::Video, "Video"},
	{MediaFileType::Subtitle, "Subtitle"},
};

struct SeasonId {
	int number{};
};

struct EpisodeId {
	int season{};
	int number{};
};
} // namespace vifo
