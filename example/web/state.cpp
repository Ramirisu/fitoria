//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria/fitoria.hpp>

using namespace fitoria;

namespace api::v1::cache {

using cache_map = std::unordered_map<std::string, std::string>;
using cache_map_ptr = std::shared_ptr<cache_map>;

namespace put {
  auto api(const http_request& req) -> net::awaitable<http_response>
  {
    auto key = req.route_params().get("key");
    auto value = req.route_params().get("value");
    if (!key || !value) {
      co_return http_response(http::status::bad_request);
    }

    auto cache = req.state<cache_map_ptr>();
    if (!cache) {
      co_return http_response(http::status::internal_server_error);
    }

    if (auto res = (*cache)->insert_or_assign(*key, *value); res.second) {
      co_return http_response(http::status::created);
    } else {
      co_return http_response(http::status::accepted);
    }
  }
}
namespace get {
  auto api(const http_request& req) -> net::awaitable<http_response>
  {
    auto key = req.route_params().get("key");
    if (!key) {
      co_return http_response(http::status::bad_request);
    }

    auto cache = req.state<cache_map_ptr>();
    if (!cache) {
      co_return http_response(http::status::internal_server_error);
    }

    if (auto it = (*cache)->find(*key); it != (*cache)->end()) {
      co_return http_response(http::status::ok).set_body(it->second);
    } else {
      co_return http_response(http::status::not_found);
    }
  }
}
}

int main()
{
  auto cache = std::make_shared<api::v1::cache::cache_map>();

  auto server = http_server::builder()
                    .route(scope("/api/v1/cache")
                               .state(cache)
                               .PUT("/{key}/{value}", api::v1::cache::put::api)
                               .GET("/{key}", api::v1::cache::get::api))
                    .build();
  server //
      .bind("127.0.0.1", 8080)
      .run();
}
