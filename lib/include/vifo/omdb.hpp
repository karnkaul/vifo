#pragma once
#include "djson/json.hpp"
#include "kcurl/http.hpp"
#include "klib/base_types.hpp"
#include "klib/enum_name.hpp"
#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace vifo {
namespace http {
using kcurl::http::Result;
} // namespace http

namespace omdb {
enum class Type : std::int8_t { Movie, Series, Episode, COUNT_ };
auto const type_map = klib::EnumNameMap<Type>{
	{Type::Movie, "movie"},
	{Type::Series, "series"},
	{Type::Episode, "episode"},
};

struct Movie {
	void serialize_to(std::string& out) const;

	std::string title{};
	int year{};
	std::string imdb_id{};
	std::string plot{};
};

struct Episode {
	void serialize_to(std::string& out) const;

	int number{};
	std::string title{};
	std::string imdb_id{};
	std::string plot{};
};

struct Season {
	void serialize_to(std::string& out) const;

	int number{};
	std::string title{};
	std::vector<Episode> episodes{};
};

struct Series {
	void serialize_to(std::string& out) const;

	std::string title{};
	int year{};
	std::string imdb_id{};
	std::string plot{};
	int total_seasons{};
};

using Payload = std::variant<Movie, Episode, Season, Series, dj::Json>;

enum class Curl : std::int8_t {
	/// \brief IService owns curl initialization/shutdown.
	Internal,
	/// \brief IService does not own curl initialization/shutdown.
	External
};

using GetApiToken = std::move_only_function<std::string_view()>;

struct Query {
	std::string_view title{};
	int season{};
	int episode{};
};

class IService : public klib::Polymorphic {
  public:
	[[nodiscard]] static auto create(GetApiToken get_api_token, Curl curl = Curl::Internal) -> std::unique_ptr<IService>;

	[[nodiscard]] virtual auto search(Query const& query, std::optional<Type> type = {}) const -> http::Result<Payload> = 0;
};

[[nodiscard]] auto serialize(Payload const& payload) -> std::string;
} // namespace omdb
} // namespace vifo
