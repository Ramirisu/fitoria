//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria_test.h>

#include <fitoria/web/scope.hpp>

using namespace fitoria::web;

TEST_SUITE_BEGIN("web.basic_scope");

TEST_CASE("methods")
{
  CHECK_EQ(std::get<0>(scope("/api") //
                           .GET("/", [](int) -> int { return 0; })
                           .routes())
               .method(),
           http::verb::get);
  CHECK_EQ(std::get<0>(scope("/api") //
                           .POST("/", [](int) -> int { return 0; })
                           .routes())
               .method(),
           http::verb::post);
  CHECK_EQ(std::get<0>(scope("/api") //
                           .PUT("/", [](int) -> int { return 0; })
                           .routes())
               .method(),
           http::verb::put);
  CHECK_EQ(std::get<0>(scope("/api") //
                           .POST("/", [](int) -> int { return 0; })
                           .routes())
               .method(),
           http::verb::post);
  CHECK_EQ(std::get<0>(scope("/api") //
                           .PATCH("/", [](int) -> int { return 0; })
                           .routes())
               .method(),
           http::verb::patch);
  CHECK_EQ(std::get<0>(scope("/api") //
                           .DELETE_("/", [](int) -> int { return 0; })
                           .routes())
               .method(),
           http::verb::delete_);
  CHECK_EQ(std::get<0>(scope("/api") //
                           .HEAD("/", [](int) -> int { return 0; })
                           .routes())
               .method(),
           http::verb::head);
  CHECK_EQ(std::get<0>(scope("/api") //
                           .OPTIONS("/", [](int) -> int { return 0; })
                           .routes())
               .method(),
           http::verb::options);
}

template <typename Next>
class adder_service {
  Next next_;
  int value_;

public:
  adder_service(Next next, int value)
      : next_(std::move(next))
      , value_(value)
  {
  }

  auto operator()(int in) const -> int
  {
    return next_(in) + value_;
  }
};

class adder {
  int value_;

public:
  adder(int value)
      : value_(value)
  {
  }

  template <typename Next>
  auto create(Next next) const
  {
    return adder_service(std::move(next), value_);
  }
};

TEST_CASE("middleware & handler")
{
  auto h = [](int i) { return i + 1; };
  auto l = adder(10);
  auto ag = adder(100);
  auto af = adder(1000);

  auto router
      = scope("/s0")
            .use(l)
            .handle(http::verb::get, "/s0h", h)
            .handle(http::verb::put, "/s0h", h)
            .sub_scope(
                scope("/s00")
                    .use(ag)
                    .handle(http::verb::get, "/s00h", h)
                    .handle(http::verb::put, "/s00h", h)
                    .sub_scope(scope("/s000")
                                   .handle(http::verb::get, "/s000h", h)
                                   .handle(http::verb::put, "/s000h", h))
                    .sub_scope(scope("/s001")
                                   .handle(http::verb::get, "/s001h", h)
                                   .handle(http::verb::put, "/s001h", h)))
            .sub_scope(
                scope("/s01")
                    .use(af)
                    .handle(http::verb::get, "/s01h", h)
                    .handle(http::verb::put, "/s01h", h)
                    .sub_scope(scope("/s010")
                                   .handle(http::verb::get, "/s010h", h)
                                   .handle(http::verb::put, "/s010h", h))
                    .sub_scope(scope("/s011")
                                   .handle(http::verb::get, "/s011h", h)
                                   .handle(http::verb::put, "/s011h", h)));

  auto services = std::apply(
      [](auto&&... routes) { return std::tuple { routes.build_service()... }; },
      router.routes());

  static_assert(std::tuple_size_v<decltype(services)> == 14);

  auto m = [](auto& route) { return std::get<0>(route); };
  auto p = [](auto& route) { return std::get<1>(route); };
  auto s = [](auto& route) { return std::get<3>(route)(0); };

  CHECK_EQ(m(std::get<0>(services)), http::verb::get);
  CHECK_EQ(p(std::get<0>(services)), "/s0/s0h");
  CHECK_EQ(s(std::get<0>(services)), 11);
  CHECK_EQ(m(std::get<1>(services)), http::verb::put);
  CHECK_EQ(p(std::get<1>(services)), "/s0/s0h");
  CHECK_EQ(s(std::get<1>(services)), 11);
  CHECK_EQ(m(std::get<2>(services)), http::verb::get);
  CHECK_EQ(p(std::get<2>(services)), "/s0/s00/s00h");
  CHECK_EQ(s(std::get<2>(services)), 111);
  CHECK_EQ(m(std::get<3>(services)), http::verb::put);
  CHECK_EQ(p(std::get<3>(services)), "/s0/s00/s00h");
  CHECK_EQ(s(std::get<3>(services)), 111);
  CHECK_EQ(m(std::get<4>(services)), http::verb::get);
  CHECK_EQ(p(std::get<4>(services)), "/s0/s00/s000/s000h");
  CHECK_EQ(s(std::get<4>(services)), 111);
  CHECK_EQ(m(std::get<5>(services)), http::verb::put);
  CHECK_EQ(p(std::get<5>(services)), "/s0/s00/s000/s000h");
  CHECK_EQ(s(std::get<5>(services)), 111);
  CHECK_EQ(m(std::get<6>(services)), http::verb::get);
  CHECK_EQ(p(std::get<6>(services)), "/s0/s00/s001/s001h");
  CHECK_EQ(s(std::get<6>(services)), 111);
  CHECK_EQ(m(std::get<7>(services)), http::verb::put);
  CHECK_EQ(p(std::get<7>(services)), "/s0/s00/s001/s001h");
  CHECK_EQ(s(std::get<7>(services)), 111);
  CHECK_EQ(m(std::get<8>(services)), http::verb::get);
  CHECK_EQ(p(std::get<8>(services)), "/s0/s01/s01h");
  CHECK_EQ(s(std::get<8>(services)), 1011);
  CHECK_EQ(m(std::get<9>(services)), http::verb::put);
  CHECK_EQ(p(std::get<9>(services)), "/s0/s01/s01h");
  CHECK_EQ(s(std::get<9>(services)), 1011);
  CHECK_EQ(m(std::get<10>(services)), http::verb::get);
  CHECK_EQ(p(std::get<10>(services)), "/s0/s01/s010/s010h");
  CHECK_EQ(s(std::get<10>(services)), 1011);
  CHECK_EQ(m(std::get<11>(services)), http::verb::put);
  CHECK_EQ(p(std::get<11>(services)), "/s0/s01/s010/s010h");
  CHECK_EQ(s(std::get<11>(services)), 1011);
  CHECK_EQ(m(std::get<12>(services)), http::verb::get);
  CHECK_EQ(p(std::get<12>(services)), "/s0/s01/s011/s011h");
  CHECK_EQ(s(std::get<12>(services)), 1011);
  CHECK_EQ(m(std::get<13>(services)), http::verb::put);
  CHECK_EQ(p(std::get<13>(services)), "/s0/s01/s011/s011h");
  CHECK_EQ(s(std::get<13>(services)), 1011);
}

TEST_SUITE_END();
