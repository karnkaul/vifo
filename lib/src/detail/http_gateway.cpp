#include "http_gateway.hpp"
#include "kcurl/http.hpp"

namespace vifo::detail {
auto HttpGateway::get_string(http::Request request, http::Query secret) const -> http::Result<std::string> {
	static auto const to_response = [](http::Response<kcurl::ByteArray> const& response) {
		return response.rewrap_as(std::string{response.payload.as_string_view()});
	};

	auto const log_request = http::to_easy_request(request);
	request.queries.push_back(std::move(secret));
	auto const easy_request = http::to_easy_request(std::move(request));
	auto const verb = [&] -> std::string_view {
		switch (request.verb) {
		case http::Verb::Get: return "GET";
		case http::Verb::Post: return "POST";
		default: return "[unknown]";
		}
	}();
	m_log.info("{} {}", verb, log_request.url);

	return http::perform(easy_request).transform(to_response);
}

auto HttpGateway::get_json(http::Request request, http::Query secret) const -> http::Result<dj::Json> {
	static auto const to_result = [](http::Response<std::string> const& response) -> http::Result<dj::Json> {
		auto json = dj::Json::parse(response.payload);
		if (!json) {
			auto const error_text = std::format("failed to parse JSON:\n{}", response.payload);
			return std::unexpected{response.rewrap_as_error(error_text)};
		}
		return response.rewrap_as(std::move(*json));
	};

	return get_string(std::move(request), std::move(secret)).and_then(to_result);
}
} // namespace vifo::detail
