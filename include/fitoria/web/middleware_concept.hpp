//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef FITORIA_WEB_MIDDLEWARE_CONCEPT_HPP
#define FITORIA_WEB_MIDDLEWARE_CONCEPT_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/core/tag_invoke.hpp>

FITORIA_NAMESPACE_BEGIN

namespace web {

inline constexpr struct new_middleware_t {
  template <typename Factory, typename Next>
    requires is_tag_invocable_v<new_middleware_t, Factory, Next>
  constexpr auto operator()(Factory&& factory, Next&& next) const
      noexcept(is_nothrow_tag_invocable_v<new_middleware_t, Factory, Next>)
  {
    return tag_invoke(new_middleware_t {},
                      std::forward<Factory>(factory),
                      std::forward<Next>(next));
  }
} new_middleware;

}

FITORIA_NAMESPACE_END

#endif
