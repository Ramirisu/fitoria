//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria/fitoria.hpp>

using namespace fitoria;
using namespace fitoria::web;

auto get_handler(http_request&) -> lazy<http_response>
{
  co_return http_response(http::status::ok);
}

auto post_handler(http_request&) -> lazy<http_response>
{
  co_return http_response(http::status::ok);
}

auto put_handler(http_request&) -> lazy<http_response>
{
  co_return http_response(http::status::ok);
}

auto patch_handler(http_request&) -> lazy<http_response>
{
  co_return http_response(http::status::ok);
}

auto delete_handler(http_request&) -> lazy<http_response>
{
  co_return http_response(http::status::ok);
}

auto head_handler(http_request&) -> lazy<http_response>
{
  co_return http_response(http::status::ok);
}

auto options_handler(http_request&) -> lazy<http_response>
{
  co_return http_response(http::status::ok);
}

auto any_handler(http_request&) -> lazy<http_response>
{
  co_return http_response(http::status::ok);
}

int main()
{
  auto server = http_server::builder()
                    // Single route by using `route`
                    .serve(route::handle<"/">(http::verb::get, get_handler))
                    .serve(route::GET<"/get">(get_handler))
                    .serve(route::POST<"/post">(post_handler))
                    .serve(route::PUT<"/put">(put_handler))
                    .serve(route::PATCH<"/patch">(patch_handler))
                    .serve(route::DELETE_<"/delete">(delete_handler))
                    .serve(route::HEAD<"/head">(head_handler))
                    .serve(route::OPTIONS<"/options">(options_handler))
                    .serve(route::any<"/any">(any_handler))
                    // Grouping routes by using `scope`
                    .serve(scope<"/api/v1">()
                               .handle<"/">(http::verb::get, get_handler)
                               .GET<"/get">(get_handler)
                               .POST<"/post">(post_handler)
                               .PUT<"/put">(put_handler)
                               .PATCH<"/patch">(patch_handler)
                               .DELETE_<"/delete">(delete_handler)
                               .HEAD<"/head">(head_handler)
                               .OPTIONS<"/options">(options_handler)
                               .any<"/any">(any_handler))
                    .build();
  server //
      .bind("127.0.0.1", 8080)
      .run();
}
