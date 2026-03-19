#include "vifo/omdb.hpp"
#include "detail/http_gateway.hpp"
#include "detail/json_io.hpp"
#include "djson/json.hpp"
#include "kcurl/curl.hpp"
#include "kcurl/http.hpp"
#include "klib/visitor.hpp"
#include "vifo/panic.hpp"
#include <string>
#include <string_view>

namespace vifo {
namespace omdb {
namespace http = kcurl::http;

namespace {
constexpr auto url_v = std::string_view{"http://www.omdbapi.com/"};

namespace key {
constexpr auto apikey_v = std::string_view{"apikey"};
constexpr auto type_v = std::string_view{"type"};
constexpr auto title_v = std::string_view{"t"};
constexpr auto season_v = std::string_view{"Season"};
constexpr auto episode_v = std::string_view{"Episode"};
} // namespace key

[[nodiscard]] constexpr auto is_valid(Type const type) { return type >= Type{0} && type < Type::COUNT_; }

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

[[nodiscard]] auto build_request(Query const& query, std::string_view type) -> kcurl::http::Request {
	return RequestBuilder{}.add_type(type).add_title(query.title).add_season(query.season).add_episode(query.episode).build();
}

template <typename Type>
[[nodiscard]] auto to_payload(http::Response<dj::Json> const& response) -> http::Response<Payload> {
	auto ret = Type{};
	detail::from_json(response.payload, ret);
	return response.rewrap_as(Payload{std::move(ret)});
}

class Service : public IService {
  public:
	explicit Service(GetApiToken get_api_token, Curl const curl) : m_get_api_token(std::move(get_api_token)) {
		if (curl == Curl::Internal) { m_curl.emplace(); }
	}

  private:
	struct Search {
		std::string_view title{};
		std::string_view type{};
		int season{};
		int episode{};
	};

	[[nodiscard]] auto search(Query const& query, std::optional<Type> type) const -> http::Result<Payload> final {
		if (!type || !is_valid(*type)) {
			return perform_search(query, {}).transform([](http::Response<dj::Json> in) { return in.rewrap_as(Payload{std::move(in.payload)}); });
		}

		auto const type_name = type_map.to_name(*type);
		switch (*type) {
		case Type::Movie: return perform_search(query, type_name).transform(&to_payload<Movie>);
		case Type::Series: {
			if (query.season > 0) { return perform_search(query, type_name).transform(&to_payload<Season>); }
			return perform_search(query, type_name).transform(&to_payload<Series>);
		}
		case Type::Episode: return perform_search(query, type_name).transform(&to_payload<Episode>);
		default: std::unreachable();
		}
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

	klib::TypedLogger<IService> m_log{};

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
	for (auto const& episode : episodes) { std::format_to(std::back_inserter(out), "  S{:02}E{:02} - {}\n", number, episode.number, episode.title); }
}

void Series::serialize_to(std::string& out) const {
	std::format_to(std::back_inserter(out), " title: {}\n year: {}\n imdb_id: {}\n total seasons: {}\n plot: {}\n", title, year, imdb_id, total_seasons, plot);
}

auto IService::create(GetApiToken get_api_token, Curl const curl) -> std::unique_ptr<IService> {
	return std::make_unique<Service>(std::move(get_api_token), curl);
}
} // namespace omdb

auto omdb::serialize(Payload const& payload) -> std::string {
	auto const visitor = klib::Visitor{
		[](dj::Json const& json) { return json.serialize(); },
		[](auto const& t) {
			auto ret = std::string{};
			t.serialize_to(ret);
			return ret;
		},
	};
	return std::visit(visitor, payload);
}
} // namespace vifo
