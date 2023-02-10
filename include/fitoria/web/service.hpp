//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef FITORIA_WEB_SERVICE_HPP
#define FITORIA_WEB_SERVICE_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/core/tag_invoke.hpp>

FITORIA_NAMESPACE_BEGIN

namespace web {

struct make_service_t {
  template <typename Factory, typename Next>
    requires tag_invocable<make_service_t, Factory, Next>
  constexpr auto operator()(Factory&& factory, Next&& next) const
      noexcept(nothrow_tag_invocable<make_service_t, Factory, Next>)
          -> tag_invoke_result_t<make_service_t, Factory, Next>
  {
    return FITORIA_NAMESPACE::tag_invoke_f(make_service_t {},
                                           std::forward<Factory>(factory),
                                           std::forward<Next>(next));
  }
};

inline constexpr make_service_t make_service {};

}

FITORIA_NAMESPACE_END

#endif
