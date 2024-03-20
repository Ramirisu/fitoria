//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria/fitoria.hpp>

using namespace fitoria;
using namespace fitoria::web;

auto get_handler(http_request&) -> net::awaitable<http_response>
{
  co_return http_response(http::status::ok);
}

auto post_handler(http_request&) -> net::awaitable<http_response>
{
  co_return http_response(http::status::ok);
}

auto put_handler(http_request&) -> net::awaitable<http_response>
{
  co_return http_response(http::status::ok);
}

auto patch_handler(http_request&) -> net::awaitable<http_response>
{
  co_return http_response(http::status::ok);
}

auto delete_handler(http_request&) -> net::awaitable<http_response>
{
  co_return http_response(http::status::ok);
}

auto head_handler(http_request&) -> net::awaitable<http_response>
{
  co_return http_response(http::status::ok);
}

auto options_handler(http_request&) -> net::awaitable<http_response>
{
  co_return http_response(http::status::ok);
}

auto any_handler(http_request&) -> net::awaitable<http_response>
{
  co_return http_response(http::status::ok);
}

int main()
{
  auto server = http_server::builder()
                    .serve(route::handle<"/">(http::verb::get, get_handler))
                    .serve(route::get<"/get">(get_handler))
                    .serve(route::post<"/post">(post_handler))
                    .serve(route::put<"/put">(put_handler))
                    .serve(route::patch<"/patch">(patch_handler))
                    .serve(route::delete_<"/delete">(delete_handler))
                    .serve(route::head<"/head">(head_handler))
                    .serve(route::options<"/options">(options_handler))
                    .serve(route::any<"/any">(any_handler))
                    .build();
  server //
      .bind("127.0.0.1", 8080)
      .run();
}
