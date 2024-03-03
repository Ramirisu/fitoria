//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria/fitoria.hpp>

using namespace fitoria;
using namespace fitoria::web;

namespace api {
namespace v1 {
  lazy<http_response> reg([[maybe_unused]] http_request& req)
  {
    co_return http_response(http::status::created);
  }

  lazy<http_response> login([[maybe_unused]] http_request& req)
  {
    co_return http_response(http::status::ok);
  }
}
namespace v2 {
  lazy<http_response> reg([[maybe_unused]] http_request& req)
  {
    co_return http_response(http::status::created);
  }

  lazy<http_response> login([[maybe_unused]] http_request& req)
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
                               .serve(route::POST<"/register">(api::v1::reg))
                               .serve(route::POST<"/login">(api::v1::login)))
                    .serve(scope<"/api/v2">()
                               .serve(route::POST<"/register">(api::v2::reg))
                               .serve(route::POST<"/login">(api::v2::login))))
            .build();
  server //
      .bind("127.0.0.1", 8080)
      .run();
}
