//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria/web.hpp>

using namespace fitoria;
using namespace fitoria::web;

auto get_handler() -> awaitable<http_response>
{
  co_return http_response::ok().build();
}

auto post_handler() -> awaitable<http_response>
{
  co_return http_response::ok().build();
}

auto put_handler() -> awaitable<http_response>
{
  co_return http_response::ok().build();
}

auto patch_handler() -> awaitable<http_response>
{
  co_return http_response::ok().build();
}

auto delete_handler() -> awaitable<http_response>
{
  co_return http_response::ok().build();
}

auto head_handler() -> awaitable<http_response>
{
  co_return http_response::ok().build();
}

auto options_handler() -> awaitable<http_response>
{
  co_return http_response::ok().build();
}

auto any_handler() -> awaitable<http_response>
{
  co_return http_response::ok().build();
}

int main()
{
  auto ioc = net::io_context();
  auto server = http_server_builder(ioc)
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
  server.bind("127.0.0.1", 8080);

  ioc.run();
}
