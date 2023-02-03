//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria_test.h>

#include <fitoria/web/basic_handler.hpp>
#include <fitoria/web/basic_middleware.hpp>
#include <fitoria/web/basic_scope.hpp>

using namespace fitoria::web;

TEST_SUITE_BEGIN("web.basic_scope");

TEST_CASE("methods")
{
  using scope_type
      = basic_scope<basic_middleware<int, int>, basic_handler<int, int>>;

  CHECK_EQ(scope_type("/api")
               .GET("/", [](int) -> int { return 0; })
               .routes()
               .at(0)
               .method(),
           http::verb::get);
  CHECK_EQ(scope_type("/api")
               .POST("/", [](int) -> int { return 0; })
               .routes()
               .at(0)
               .method(),
           http::verb::post);
  CHECK_EQ(scope_type("/api")
               .PUT("/", [](int) -> int { return 0; })
               .routes()
               .at(0)
               .method(),
           http::verb::put);
  CHECK_EQ(scope_type("/api")
               .POST("/", [](int) -> int { return 0; })
               .routes()
               .at(0)
               .method(),
           http::verb::post);
  CHECK_EQ(scope_type("/api")
               .PATCH("/", [](int) -> int { return 0; })
               .routes()
               .at(0)
               .method(),
           http::verb::patch);
  CHECK_EQ(scope_type("/api")
               .DELETE_("/", [](int) -> int { return 0; })
               .routes()
               .at(0)
               .method(),
           http::verb::delete_);
  CHECK_EQ(scope_type("/api")
               .HEAD("/", [](int) -> int { return 0; })
               .routes()
               .at(0)
               .method(),
           http::verb::head);
  CHECK_EQ(scope_type("/api")
               .OPTIONS("/", [](int) -> int { return 0; })
               .routes()
               .at(0)
               .method(),
           http::verb::options);
}

TEST_CASE("middleware & handler")
{
  using scope_type
      = basic_scope<basic_middleware<int, int>, basic_handler<int, int>>;

  auto h = [](int) -> int { return 0; };
  auto l = [](int) -> int { return 1; };
  auto ag = [](int) -> int { return 2; };
  auto af = [](int) -> int { return 3; };

  auto rg
      = scope_type("/ramirisu")
            .use(l)
            .route(http::verb::get, "/libraries", h)
            .route(http::verb::put, "/libraries", h)
            .sub_scope(
                scope_type("/gul")
                    .use(ag)
                    .route(http::verb::get, "/tags", h)
                    .route(scope_type::route_type(http::verb::put, "/tags", h))
                    .sub_scope(scope_type("/tags")
                                   .route(http::verb::get, "/{tag}", h)
                                   .route(http::verb::put, "/{tag}", h))
                    .sub_scope(scope_type("/branches")
                                   .route(http::verb::get, "/{branch}", h)
                                   .route(http::verb::put, "/{branch}", h)))
            .sub_scope(
                scope_type("/fitoria")
                    .use(af)
                    .route(http::verb::get, "/tags", h)
                    .route(scope_type::route_type(http::verb::put, "/tags", h))
                    .sub_scope(scope_type("/tags")
                                   .route(http::verb::get, "/{tag}", h)
                                   .route(http::verb::put, "/{tag}", h))
                    .sub_scope(scope_type("/branches")
                                   .route(http::verb::get, "/{branch}", h)
                                   .route(http::verb::put, "/{branch}", h)));

  const auto exp = std::vector<scope_type::route_type> {
    scope_type::route_type(http::verb::get, "/ramirisu/libraries", h).use(l),
    scope_type::route_type(http::verb::put, "/ramirisu/libraries", h).use(l),
    scope_type::route_type(http::verb::get, "/ramirisu/gul/tags", h)
        .use(l)
        .use(ag),
    scope_type::route_type(http::verb::put, "/ramirisu/gul/tags", h)
        .use(l)
        .use(ag),
    scope_type::route_type(http::verb::get, "/ramirisu/gul/tags/{tag}", h)
        .use(l)
        .use(ag),
    scope_type::route_type(http::verb::put, "/ramirisu/gul/tags/{tag}", h)
        .use(l)
        .use(ag),
    scope_type::route_type(http::verb::get, "/ramirisu/gul/branches/{branch}",
                           h)
        .use(l)
        .use(ag),
    scope_type::route_type(http::verb::put, "/ramirisu/gul/branches/{branch}",
                           h)
        .use(l)
        .use(ag),
    scope_type::route_type(http::verb::get, "/ramirisu/fitoria/tags", h)
        .use(l)
        .use(af),
    scope_type::route_type(http::verb::put, "/ramirisu/fitoria/tags", h)
        .use(l)
        .use(af),
    scope_type::route_type(http::verb::get, "/ramirisu/fitoria/tags/{tag}", h)
        .use(l)
        .use(af),
    scope_type::route_type(http::verb::put, "/ramirisu/fitoria/tags/{tag}", h)
        .use(l)
        .use(af),
    scope_type::route_type(http::verb::get,
                           "/ramirisu/fitoria/branches/{branch}", h)
        .use(l)
        .use(af),
    scope_type::route_type(http::verb::put,
                           "/ramirisu/fitoria/branches/{branch}", h)
        .use(l)
        .use(af)

  };
  CHECK(range_equal(rg.routes(), exp, [](auto& lhs, auto& rhs) {
    if (lhs.method() == rhs.method() && lhs.path() == rhs.path()
        && lhs.handler()(0) == rhs.handler()(0)) {
      for (std::size_t i = 0; i < lhs.middlewares().size(); ++i) {
        if (lhs.middlewares()[i](0) != rhs.middlewares()[i](0)) {
          return false;
        }
      }
      return true;
    }
    return false;
  }));
}

TEST_SUITE_END();
