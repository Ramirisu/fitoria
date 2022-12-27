//
// Copyright (c) 2022 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria_test.h>

#include <fitoria/core/utility.hpp>
#include <fitoria/http_server/handlers_invoker.hpp>

using namespace fitoria;

TEST_SUITE_BEGIN("handlers_invoker");

namespace {

class context;

struct test_handler_trait {
  using handler_t = std::function<void(context&)>;
  using handlers_t = std::vector<handler_t>;
  using handler_result_t = typename function_traits<handler_t>::result_type;
};

class context {
public:
  context(handlers_invoker<test_handler_trait> chain)
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
  handlers_invoker<test_handler_trait> chain_;
};
}

TEST_CASE("handlers invocation order")
{
  int state = 0;
  typename test_handler_trait::handlers_t handlers {
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
