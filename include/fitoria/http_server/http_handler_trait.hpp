//
// Copyright (c) 2022 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include <fitoria/core/config.hpp>

#include <fitoria/core/fwd.hpp>

#include <fitoria/core/detail/utility.hpp>

#include <fitoria/core/handler_concept.hpp>
#include <fitoria/core/net.hpp>
#include <fitoria/core/utility.hpp>

#include <fitoria/http_server/http_error.hpp>
#include <fitoria/http_server/http_response.hpp>

FITORIA_NAMESPACE_BEGIN

struct http_handler_trait {
private:
  using result_t = net::awaitable<expected<http_response, http_error>>;

public:
  using handler_t
      = detail::repeated_input_variant_function_t<result_t, http_request&, 5>;
  using handlers_t = std::vector<handler_t>;
  using handler_result_t = result_t;
  struct handler_compare_t;
};

FITORIA_NAMESPACE_END
