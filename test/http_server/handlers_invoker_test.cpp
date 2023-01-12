//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria_test.h>

#include <fitoria/core/type_traits.hpp>

#include <fitoria/http_server/handlers_invoker.hpp>

using namespace fitoria;

TEST_SUITE_BEGIN("handlers_invoker");

namespace {

class context;
class request;

struct test_handler_trait {
  using middleware_input_param_t = context&;
  using middleware_t = std::function<void(context&)>;
  using middleware_result_t =
      typename function_traits<middleware_t>::result_type;
  using middlewares_t = std::vector<middleware_t>;

  using handler_input_param_t = request&;
  using handler_t = std::function<void(request&)>;
  using handler_result_t = typename function_traits<handler_t>::result_type;
};

class request { };

class context {
public:
  context(handlers_invoker<test_handler_trait> chain)
      : chain_(std::move(chain))
  {
  }

  void next()
  {
    chain_.next(*this);
  }

  operator request&() noexcept
  {
    return req_;
  }

private:
  handlers_invoker<test_handler_trait> chain_;
  request req_;
};
}

TEST_CASE("handlers invocation order")
{
  int state = 0;
  typename test_handler_trait::middlewares_t middlewares {
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

  typename test_handler_trait::handler_t handlers {
    [&](request&) { CHECK_EQ(++state, 3); },
  };

  context ctx(handlers_invoker<test_handler_trait>(middlewares, handlers));
  ctx.next();
  CHECK_EQ(++state, 6);
}

TEST_SUITE_END();
