#pragma once
#include "klib/log.hpp"
#include <djson/json.hpp>
#include <kcurl/http.hpp>

namespace vifo::detail {
namespace http = kcurl::http;

class HttpGateway {
  public:
	[[nodiscard]] auto get_string(http::Request request, http::Query secret) const -> http::Result<std::string>;
	[[nodiscard]] auto get_json(http::Request request, http::Query secret) const -> http::Result<dj::Json>;

	klib::TypedLogger<HttpGateway> m_log{};
};
} // namespace vifo::detail
