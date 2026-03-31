#pragma once
#include "command/command.hpp"
#include "djson/json.hpp"
#include "klib/ptr.hpp"
#include "vifo/formatter/formatter.hpp"
#include "vifo/formatter/movie.hpp"
#include "vifo/formatter/season.hpp"
#include "vifo/formatter/series.hpp"
#include "vifo/json_io.hpp"
#include "vifo/omdb.hpp"
#include "vifo/types.hpp"
#include <string_view>

namespace vifo::cli::command {
class OmdbBase : public Command {
  public:
	explicit OmdbBase(omdb::IService const& omdb_service, std::string_view name);

  protected:
	template <typename FormatterT>
	[[nodiscard]] static constexpr auto to_name() -> std::string_view {
		if constexpr (std::same_as<FormatterT, formatter::Movie>) {
			return "movie";
		} else if constexpr (std::same_as<FormatterT, formatter::Season>) {
			return "season";
		} else if constexpr (std::same_as<FormatterT, formatter::Series>) {
			return "series";
		} else {
			return "unknown";
		}
	}

	virtual void read_from_json() = 0;
	virtual auto create_formatter() -> ExitCode = 0;

	klib::Ptr<omdb::IService const> m_omdb_service{};
	std::unique_ptr<IFormatter> m_formatter{};

	dj::Json m_json{};

  private:
	[[nodiscard]] auto get_name() const -> std::string_view final { return m_command_name; }
	[[nodiscard]] auto get_help() const -> std::string_view final { return m_command_help; }
	auto get_parameters() -> std::vector<clap::Parameter> final;
	auto execute() -> ExitCode final;

	[[nodiscard]] auto execute_stub(IFormatter& formatter) -> ExitCode;

	std::string_view m_command_name{};
	std::string m_command_help{};
	std::string m_json_help{};
	std::string m_directory_help{};

	std::string_view m_format_json{};
	std::string_view m_directory{"."};
};

template <std::derived_from<IFormatter> FormatterT>
class Omdb : public OmdbBase {
  public:
	explicit Omdb(omdb::IService const& omdb_service) : OmdbBase(omdb_service, to_name<FormatterT>()) {}

  private:
	void read_from_json() final { from_json(m_json, m_format); }

	auto create_formatter() -> ExitCode final {
		auto formatter = FormatterT::create(*m_omdb_service, m_format);
		if (!formatter) { return handle_error(formatter.error()); }
		m_formatter = std::make_unique<FormatterT>(std::move(*formatter));
		return ExitCode::Success;
	}

	FormatterT::Format m_format{};
};

using Movie = Omdb<formatter::Movie>;
using Season = Omdb<formatter::Season>;
using Series = Omdb<formatter::Series>;
} // namespace vifo::cli::command
