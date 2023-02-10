//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef FITORIA_WEB_HANDLER_HPP
#define FITORIA_WEB_HANDLER_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/core/net.hpp>
#include <fitoria/core/type_traits.hpp>

#include <fitoria/web/http_context.hpp>
#include <fitoria/web/http_request.hpp>
#include <fitoria/web/http_response.hpp>
#include <fitoria/web/service.hpp>

FITORIA_NAMESPACE_BEGIN

namespace web {

template <typename Next>
class handler_service {
  friend class handler;

public:
  auto operator()(http_context& ctx) const -> net::awaitable<http_response>
  {
    return next_(static_cast<http_request&>(ctx));
  }

private:
  template <typename Next2>
  handler_service(Next2&& next)
      : next_(std::forward<Next2>(next))
  {
    static_assert(std::is_invocable_r_v<net::awaitable<http_response>, Next2,
                                        http_request&>);
  }

  Next next_;
};

template <typename Next>
handler_service(Next&&) -> handler_service<std::decay_t<Next>>;

class handler {
  template <typename Next>
  auto new_service(Next&& next) const
  {
    return handler_service(std::forward<Next>(next));
  }

public:
  template <uncvref_same_as<handler> Self, typename Next>
  friend constexpr auto tag_invoke(make_service_t, Self&& self, Next&& next)
  {
    return std::forward<Self>(self).new_service(std::forward<Next>(next));
  }
};
}

FITORIA_NAMESPACE_END

#endif
