//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria/fitoria.hpp>

#include <shared_mutex>

using namespace fitoria;
using namespace fitoria::web;

namespace cache {

class simple_cache {
  using map_type = unordered_string_map<std::string>;

public:
  optional<map_type::mapped_type> get(std::string_view key) const
  {
    auto lock = std::shared_lock { mutex_ };
    if (auto it = map_.find(key); it != map_.end()) {
      return it->second;
    }

    return nullopt;
  }

  bool put(const std::string& key, std::string value)
  {
    auto lock = std::unique_lock { mutex_ };
    return map_.insert_or_assign(key, std::move(value)).second;
  }

private:
  map_type map_;
  mutable std::shared_mutex mutex_;
};

using simple_cache_ptr = std::shared_ptr<simple_cache>;

namespace put {
  auto api(const http_request& req) -> lazy<http_response>
  {
    auto key = req.params().get("key");
    auto value = req.params().get("value");
    if (!key || !value) {
      co_return http_response(http::status::bad_request);
    }

    auto cache = req.state<simple_cache_ptr>();
    if (!cache) {
      co_return http_response(http::status::internal_server_error);
    }

    if ((*cache)->put(*key, *value)) {
      co_return http_response(http::status::created);
    } else {
      co_return http_response(http::status::accepted);
    }
  }
}
namespace get {
  auto api(const http_request& req) -> lazy<http_response>
  {
    auto key = req.params().get("key");
    if (!key) {
      co_return http_response(http::status::bad_request);
    }

    auto cache = req.state<simple_cache_ptr>();
    if (!cache) {
      co_return http_response(http::status::internal_server_error);
    }

    if (auto value = (*cache)->get(*key); value) {
      co_return http_response(http::status::ok)
          .set_field(http::field::content_type,
                     http::fields::content_type::plaintext())
          .set_body(*value);
    } else {
      co_return http_response(http::status::not_found);
    }
  }
}
}

int main()
{
  auto cache = std::make_shared<cache::simple_cache_ptr>();

  auto server
      = http_server::builder()
            .serve(scope<"/cache">()
                       .state(cache)
                       .serve(route::PUT<"/{key}/{value}">(cache::put::api))
                       .serve(route::GET<"/{key}">(cache::get::api)))
            .build();
  server //
      .bind("127.0.0.1", 8080)
      .run();
}
