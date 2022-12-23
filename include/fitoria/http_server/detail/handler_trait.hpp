//
// Copyright (c) 2022 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include <fitoria/core/config.hpp>

#include <fitoria/core/fwd.hpp>

#include <fitoria/core/handler_concept.hpp>
#include <fitoria/core/net.hpp>
#include <fitoria/core/utility.hpp>

#include <functional>

FITORIA_NAMESPACE_BEGIN

namespace detail {

struct handler_trait {
  using handler_t = std::function<net::awaitable<void>(http_context&)>;
  using handlers_t = std::vector<handler_t>;
  using handler_result_t = typename function_traits<handler_t>::result_type;
  static constexpr bool handler_result_awaitable = true;
  struct handler_compare_t;
};

}

FITORIA_NAMESPACE_END
