//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria/fitoria.hpp>

using namespace fitoria;

auto get(http_request&) -> net::awaitable<http_response>
{
  co_return http_response(http::status::ok);
}

auto post(http_request&) -> net::awaitable<http_response>
{
  co_return http_response(http::status::ok);
}

auto put(http_request&) -> net::awaitable<http_response>
{
  co_return http_response(http::status::ok);
}

auto patch(http_request&) -> net::awaitable<http_response>
{
  co_return http_response(http::status::ok);
}

auto delete_(http_request&) -> net::awaitable<http_response>
{
  co_return http_response(http::status::ok);
}

auto head(http_request&) -> net::awaitable<http_response>
{
  co_return http_response(http::status::ok);
}

auto options(http_request&) -> net::awaitable<http_response>
{
  co_return http_response(http::status::ok);
}

int main()
{
  auto server = http_server::builder()
                    // Single route by using `route`
                    .route(route(http::verb::get, "/", get))
                    .route(route::GET("/get", get))
                    .route(route::POST("/post", post))
                    .route(route::PUT("/put", put))
                    .route(route::PATCH("/patch", patch))
                    .route(route::DELETE_("/delete", delete_))
                    .route(route::HEAD("/head", head))
                    .route(route::OPTIONS("/options", options))
                    // Grouping routes by using `scope`
                    .route(scope("/api/v1")
                               .route(route(http::verb::get, "/", get))
                               .GET("/get", get)
                               .POST("/post", post)
                               .PUT("/put", put)
                               .PATCH("/patch", patch)
                               .DELETE_("/delete", delete_)
                               .HEAD("/head", head)
                               .OPTIONS("/options", options))
                    .build();
  server //
      .bind("127.0.0.1", 8080)
      .run();
}
