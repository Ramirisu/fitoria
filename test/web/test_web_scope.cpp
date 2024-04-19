//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria/test/test.hpp>

#include <fitoria/web/scope.hpp>
#include <fitoria/web/to_middleware.hpp>

using namespace fitoria;
using namespace fitoria::web;

TEST_SUITE_BEGIN("[fitoria.web.scope]");

TEST_CASE("method")
{
  using req_type = int;
  using res_type = int;
  auto handler = [](req_type) -> res_type { return 0; };
  auto m = [](auto&& scope) {
    return std::get<0>(scope.routes())
        .template build<req_type, res_type>()
        .method();
  };

  CHECK_EQ(
      m(scope<"/api">().serve(route::handle<"/">(http::verb::get, handler))),
      http::verb::get);
  CHECK_EQ(m(scope<"/api">().serve(route::get<"/">(handler))), http::verb::get);
  CHECK_EQ(m(scope<"/api">().serve(route::post<"/">(handler))),
           http::verb::post);
  CHECK_EQ(m(scope<"/api">().serve(route::put<"/">(handler))), http::verb::put);
  CHECK_EQ(m(scope<"/api">().serve(route::patch<"/">(handler))),
           http::verb::patch);
  CHECK_EQ(m(scope<"/api">().serve(route::delete_<"/">(handler))),
           http::verb::delete_);
  CHECK_EQ(m(scope<"/api">().serve(route::head<"/">(handler))),
           http::verb::head);
  CHECK_EQ(m(scope<"/api">().serve(route::options<"/">(handler))),
           http::verb::options);
  CHECK_EQ(m(scope<"/api">().serve(route::any<"/">(handler))),
           http::verb::unknown);
}

template <typename Request, typename Response, typename Next>
class adder_middleware {
  friend class adder;

public:
  auto operator()(Request in) const -> Response
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

class adder {
public:
  adder(int value)
      : value_(value)
  {
  }

  template <typename Request,
            typename Response,
            decay_to<adder> Self,
            typename Next>
  friend auto
  tag_invoke(to_middleware_t<Request, Response>, Self&& self, Next&& next)
  {
    return std::forward<Self>(self)
        .template to_middleware_impl<Request, Response>(
            std::forward<Next>(next));
  }

private:
  template <typename Request, typename Response, typename Next>
  auto to_middleware_impl(Next&& next) const
  {
    return adder_middleware<Request, Response, std::decay_t<Next>>(
        std::forward<Next>(next), value_);
  }

  int value_;
};

TEST_CASE("middleware & handler")
{
  using req_type = int;
  using res_type = int;
  auto h = [](req_type i) -> res_type { return i + 1; };
  auto l = adder(10);
  auto ag = adder(100);
  auto af = adder(1000);

  auto router = scope<"/s0">()
                    .use(l)
                    .serve(route::get<"/s0h">(h))
                    .serve(route::put<"/s0h">(h))
                    .serve(scope<"/s00">()
                               .use(ag)
                               .serve(route::get<"/s00h">(h))
                               .serve(route::put<"/s00h">(h))
                               .serve(scope<"/s000">()
                                          .serve(route::get<"/s000h">(h))
                                          .serve(route::put<"/s000h">(h)))
                               .serve(scope<"/s001">()
                                          .serve(route::get<"/s001h">(h))
                                          .serve(route::put<"/s001h">(h))))
                    .serve(scope<"/s01">()
                               .use(af)
                               .serve(route::get<"/s01h">(h))
                               .serve(route::put<"/s01h">(h))
                               .serve(scope<"/s010">()
                                          .serve(route::get<"/s010h">(h))
                                          .serve(route::put<"/s010h">(h)))
                               .serve(scope<"/s011">()
                                          .serve(route::get<"/s011h">(h))
                                          .serve(route::put<"/s011h">(h))));

  auto services = std::apply(
      [](auto&&... routes) {
        return std::tuple { routes.template build<req_type, res_type>(
            adder(0))... };
      },
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
