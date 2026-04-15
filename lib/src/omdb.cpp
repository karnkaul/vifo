#include "vifo/omdb.hpp"
#include "detail/http_gateway.hpp"
#include "djson/json.hpp"
#include "kcurl/curl.hpp"
#include "kcurl/http.hpp"
#include "klib/enum/name.hpp"
#include "vifo/json_io.hpp"
#include "vifo/panic.hpp"
#include "vifo/types.hpp"
#include <string>
#include <string_view>

namespace vifo::omdb {
namespace http = kcurl::http;

namespace {
constexpr auto url_v = std::string_view{"http://www.omdbapi.com/"};

enum class Type : std::int8_t { Movie, Series, Episode, COUNT_ };
auto const type_name_map = klib::EnumNameMap<Type>{
	{Type::Movie, "movie"},
	{Type::Series, "series"},
	{Type::Episode, "episode"},
};

namespace key {
constexpr auto apikey_v = std::string_view{"apikey"};
constexpr auto type_v = std::string_view{"type"};
constexpr auto title_v = std::string_view{"t"};
constexpr auto season_v = std::string_view{"Season"};
constexpr auto episode_v = std::string_view{"Episode"};
} // namespace key

struct RequestBuilder {
	auto add_query(std::string_view const key, std::string value) -> RequestBuilder& {
		if (!value.empty()) { request.queries.push_back(http::Query{.key = std::string{key}, .value = std::move(value)}); }
		return *this;
	}

	auto add_type(std::string_view const type) -> RequestBuilder& {
		if (type.empty()) { return *this; }
		return add_query(key::type_v, std::string{type});
	}

	auto add_title(std::string_view const title) -> RequestBuilder& { return add_query(key::title_v, http::escape(title)); }

	auto add_season(int const season) -> RequestBuilder& {
		if (season <= 0) { return *this; }
		return add_query(key::season_v, std::format("{}", season));
	}

	auto add_episode(int const episode) -> RequestBuilder& {
		if (episode <= 0) { return *this; }
		return add_query(key::episode_v, std::format("{}", episode));
	}

	auto build() -> http::Request { return std::move(request); }

	http::Request request{.base_url = std::string{url_v}};
};

template <typename T>
[[nodiscard]] auto to_type(http::Response<dj::Json> const& response) -> http::Response<T> {
	auto ret = T{};
	from_json(response.payload, ret);
	return response.rewrap_as(std::move(ret));
}

class Service : public IService {
  public:
	explicit Service(GetApiToken get_api_token, Curl const curl) : m_get_api_token(std::move(get_api_token)) {
		if (curl == Curl::Internal) { m_curl.emplace(); }
	}

  private:
	struct Query {
		std::string_view title{};
		int season{};
		int episode{};
	};

	[[nodiscard]] static auto build_request(Query const& query, std::string_view type) -> kcurl::http::Request {
		return RequestBuilder{}.add_type(type).add_title(query.title).add_season(query.season).add_episode(query.episode).build();
	}

	[[nodiscard]] auto search_generic(std::string_view const title) const -> http::Result<dj::Json> final {
		auto const query = Query{.title = title};
		return perform_search(query, {});
	}

	[[nodiscard]] auto search_movie(std::string_view const title) const -> http::Result<Movie> final {
		auto const query = Query{.title = title};
		return perform_search(query, type_name_map.to_name(Type::Movie)).transform(&to_type<Movie>);
	}

	[[nodiscard]] auto search_episode(std::string_view const title, int const season, int const episode) const -> http::Result<Episode> final {
		auto const query = Query{.title = title, .season = season, .episode = episode};
		return perform_search(query, type_name_map.to_name(Type::Episode)).transform(&to_type<Episode>);
	}

	[[nodiscard]] auto search_season(std::string_view const title, int const season) const -> http::Result<Season> final {
		auto const query = Query{.title = title, .season = season};
		return perform_search(query, type_name_map.to_name(Type::Series)).transform(&to_type<Season>);
	}

	[[nodiscard]] auto search_series(std::string_view const title) const -> http::Result<Series> final {
		auto const query = Query{.title = title};
		return perform_search(query, type_name_map.to_name(Type::Series)).transform(&to_type<Series>);
	}

	[[nodiscard]] auto create_secret() const -> kcurl::http::Query {
		auto const token = m_get_api_token();
		if (token.empty()) { throw Panic{"invalid (empty) omdb API token"}; }

		return http::Query{
			.key = std::string{key::apikey_v},
			.value = std::string{token},
		};
	}

	[[nodiscard]] auto perform_search(Query const& query, std::string_view type) const -> http::Result<dj::Json> {
		return m_gateway.get_json(build_request(query, type), create_secret());
	}

	klib::log::Typed<IService> m_log{};

	std::optional<kcurl::Curl> m_curl{};
	detail::HttpGateway m_gateway{};
	mutable GetApiToken m_get_api_token{};
};
} // namespace

void Movie::serialize_to(std::string& out) const {
	std::format_to(std::back_inserter(out), "movie:\n title: {}\n year: {}\n imdb_id: {}\n plot: {}", title, year, imdb_id, plot);
}

void Episode::serialize_to(std::string& out) const {
	std::format_to(std::back_inserter(out), "episode:\n title: {}\n number: {}\n imdb_id: {}\n plot: {}", title, number, imdb_id, plot);
}

void Season::serialize_to(std::string& out) const {
	std::format_to(std::back_inserter(out), " title: {}\n number: {}\n episodes:\n", title, number);
	for (auto const& episode : episodes) {
		auto const id = EpisodeId{.season = number, .number = episode.number};
		std::format_to(std::back_inserter(out), "  {} - {}\n", id.format(), episode.title);
	}
}

void Series::serialize_to(std::string& out) const {
	std::format_to(std::back_inserter(out), " title: {}\n year: {}\n imdb_id: {}\n total seasons: {}\n plot: {}\n", title, year, imdb_id, total_seasons, plot);
}

auto IService::create(GetApiToken get_api_token, Curl const curl) -> std::unique_ptr<IService> {
	return std::make_unique<Service>(std::move(get_api_token), curl);
}
} // namespace vifo::omdb
