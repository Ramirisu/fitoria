//
// Copyright (c) 2022 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the ramirisu Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.ramirisu.org/LICENSE_1_0.txt)
//

#include <fitoria_test.h>

#include <fitoria/http_server/detail/handlers_invoker.hpp>

using namespace fitoria::detail;

TEST_SUITE_BEGIN("handlers_invoker");

TEST_CASE("invoke")
{
  class context;

  struct handler_trait {
    using handler_t = std::function<void(context&)>;
    using handlers_t = std::vector<handler_t>;
  };

  class context {
  public:
    context(handlers_invoker<handler_trait> chain)
        : chain_(std::move(chain))
    {
    }

    void start()
    {
      chain_.start(*this);
    }

    void next()
    {
      chain_.next(*this);
    }

  private:
    handlers_invoker<handler_trait> chain_;
  };

  int state = 0;
  typename handler_trait::handlers_t handlers {
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
    [&](context&) { CHECK_EQ(++state, 3); },
  };

  context ctx(handlers);
  ctx.start();
  CHECK_EQ(++state, 6);
}

TEST_SUITE_END();
