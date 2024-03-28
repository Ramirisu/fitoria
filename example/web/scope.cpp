//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria/web.hpp>

using namespace fitoria;
using namespace fitoria::web;

namespace api {
namespace v1 {
  auto reg(http_request&) -> net::awaitable<http_response>
  {
    co_return http_response(http::status::created);
  }

  auto login(http_request&) -> net::awaitable<http_response>
  {
    co_return http_response(http::status::ok);
  }
}
namespace v2 {
  auto reg(http_request&) -> net::awaitable<http_response>
  {
    co_return http_response(http::status::created);
  }

  auto login(http_request&) -> net::awaitable<http_response>
  {
    co_return http_response(http::status::ok);
  }
}
}

int main()
{
  auto server
      = http_server::builder()
            .serve(
                scope<>()
                    .use(middleware::logger())
                    .serve(scope<"/api/v1">()
                               .serve(route::post<"/register">(api::v1::reg))
                               .serve(route::post<"/login">(api::v1::login)))
                    .serve(scope<"/api/v2">()
                               .serve(route::post<"/register">(api::v2::reg))
                               .serve(route::post<"/login">(api::v2::login))))
            .build();
  server //
      .bind("127.0.0.1", 8080)
      .run();
}
