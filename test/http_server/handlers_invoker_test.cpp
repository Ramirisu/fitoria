//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria_test.h>

#include <fitoria/core/type_traits.hpp>

#include <fitoria/http_server/basic_handler.hpp>
#include <fitoria/http_server/basic_middleware.hpp>
#include <fitoria/http_server/handlers_invoker.hpp>

using namespace fitoria;

TEST_SUITE_BEGIN("handlers_invoker");

namespace {

class context;
class request;

using middleware_type = basic_middleware<context&, void>;
using handler_type = basic_handler<request&, void>;

class request { };

class context {
public:
  using invoker_type = handlers_invoker<middleware_type, handler_type>;

  context(invoker_type invoker)
      : invoker_(std::move(invoker))
  {
  }

  void next()
  {
    invoker_.next(*this);
  }

  operator request&() noexcept
  {
    return req_;
  }

private:
  invoker_type invoker_;
  request req_;
};
}

TEST_CASE("handlers invocation order")
{
  int state = 0;
  std::vector<middleware_type> middlewares {
    [&](context& ctx) {
      CHECK_EQ(++state, 1);
      ctx.next();
      CHECK_EQ(++state, 5);
    },
    [&](context& ctx) {
      CHECK_EQ(++state, 2);
      ctx.next();
      CHECK_EQ(++state, 4);
    },
  };

  handler_type handlers {
    [&](request&) { CHECK_EQ(++state, 3); },
  };

  context ctx(context::invoker_type(middlewares, handlers));
  ctx.next();
  CHECK_EQ(++state, 6);
}

TEST_SUITE_END();
