//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include <fitoria/core/config.hpp>

#include <fitoria/core/fwd.hpp>

#include <fitoria/core/handler_concept.hpp>
#include <fitoria/core/net.hpp>
#include <fitoria/core/variant_function.hpp>

#include <fitoria/http_server/http_response.hpp>

FITORIA_NAMESPACE_BEGIN

struct http_handler_trait {
private:
  using result_t = http_response;

public:
  using middleware_input_param_t = http_context&;
  using middleware_t
      = std::function<net::awaitable<result_t>(middleware_input_param_t)>;
  using middleware_result_t = net::awaitable<result_t>;

  using middlewares_t = std::vector<middleware_t>;

  using handler_input_param_t = http_request&;
  using handler_t
      = variant_function_t<net::awaitable<result_t>, handler_input_param_t, 5>;
  using handler_result_t = net::awaitable<result_t>;
  struct handler_compare_t;
};

FITORIA_NAMESPACE_END
