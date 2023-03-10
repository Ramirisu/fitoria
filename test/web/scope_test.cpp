//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria_test.h>

#include <fitoria/web/middleware_concept.hpp>
#include <fitoria/web/scope.hpp>

using namespace fitoria;
using namespace fitoria::web;

TEST_SUITE_BEGIN("web.scope");

TEST_CASE("method")
{
  auto method = [](auto&& scope) {
    return std::get<0>(scope.routes()).build().method();
  };
  CHECK_EQ(method(scope<"/api">().GET<"/">([](int) -> int { return 0; })),
           http::verb::get);
  CHECK_EQ(method(scope<"/api">().POST<"/">([](int) -> int { return 0; })),
           http::verb::post);
  CHECK_EQ(method(scope<"/api">().PUT<"/">([](int) -> int { return 0; })),
           http::verb::put);
  CHECK_EQ(method(scope<"/api">().POST<"/">([](int) -> int { return 0; })),
           http::verb::post);
  CHECK_EQ(method(scope<"/api">().PATCH<"/">([](int) -> int { return 0; })),
           http::verb::patch);
  CHECK_EQ(method(scope<"/api">().DELETE_<"/">([](int) -> int { return 0; })),
           http::verb::delete_);
  CHECK_EQ(method(scope<"/api">().HEAD<"/">([](int) -> int { return 0; })),
           http::verb::head);
  CHECK_EQ(method(scope<"/api">().OPTIONS<"/">([](int) -> int { return 0; })),
           http::verb::options);
}

template <typename Next>
class adder_middleware {
  friend class adder;

public:
  auto operator()(int in) const -> int
  {
    return next_(in) + value_;
  }

private:
  template <typename Next2>
  adder_middleware(Next2&& next, int value)
      : next_(std::forward<Next2>(next))
      , value_(value)
  {
  }

  Next next_;
  int value_;
};

template <typename Next>
adder_middleware(Next&&, int) -> adder_middleware<std::decay_t<Next>>;

class adder {
public:
  adder(int value)
      : value_(value)
  {
  }

  template <uncvref_same_as<adder> Self, typename Next>
  friend constexpr auto tag_invoke(new_middleware_t, Self&& self, Next&& next)
  {
    return std::forward<Self>(self).new_middleware_impl(
        std::forward<Next>(next));
  }

private:
  template <typename Next>
  auto new_middleware_impl(Next&& next) const
  {
    return adder_middleware(std::forward<Next>(next), value_);
  }

  int value_;
};

TEST_CASE("middleware & handler")
{
  auto h = [](int i) { return i + 1; };
  auto l = adder(10);
  auto ag = adder(100);
  auto af = adder(1000);

  auto router
      = scope<"/s0">()
            .use(l)
            .handle<"/s0h">(http::verb::get, h)
            .handle<"/s0h">(http::verb::put, h)
            .sub_scope(
                scope<"/s00">()
                    .use(ag)
                    .handle<"/s00h">(http::verb::get, h)
                    .handle<"/s00h">(http::verb::put, h)
                    .sub_scope(scope<"/s000">()
                                   .handle<"/s000h">(http::verb::get, h)
                                   .handle<"/s000h">(http::verb::put, h))
                    .sub_scope(scope<"/s001">()
                                   .handle<"/s001h">(http::verb::get, h)
                                   .handle<"/s001h">(http::verb::put, h)))
            .sub_scope(
                scope<"/s01">()
                    .use(af)
                    .handle<"/s01h">(http::verb::get, h)
                    .handle<"/s01h">(http::verb::put, h)
                    .sub_scope(scope<"/s010">()
                                   .handle<"/s010h">(http::verb::get, h)
                                   .handle<"/s010h">(http::verb::put, h))
                    .sub_scope(scope<"/s011">()
                                   .handle<"/s011h">(http::verb::get, h)
                                   .handle<"/s011h">(http::verb::put, h)));

  auto services = std::apply(
      [](auto&&... routes) { return std::tuple { routes.build(adder(0))... }; },
      router.routes());

  static_assert(std::tuple_size_v<decltype(services)> == 14);

  auto m = [](auto& route) { return route.method(); };
  auto p = [](auto& route) { return route.matcher().pattern(); };
  auto s = [](auto& route) { return route.service()(0); };

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
