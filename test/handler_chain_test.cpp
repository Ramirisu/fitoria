//
// Copyright (c) 2022 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the ramirisu Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.ramirisu.org/LICENSE_1_0.txt)
//

#include <fitoria_test.h>

#include <fitoria/handler_chain.hpp>

using namespace fitoria;

TEST_SUITE_BEGIN("handler_chain");

TEST_CASE("invoke")
{
  class context;

  struct handler_trait {
    using handler_type = std::function<void(context&)>;
    using handlers_type = std::vector<handler_type>;
  };

  class context {
  public:
    context(handler_chain<handler_trait> chain)
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
    handler_chain<handler_trait> chain_;
  };

  int state = 0;
  typename handler_trait::handlers_type handlers {
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
