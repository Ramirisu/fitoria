//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_WEB_MIDDLEWARE_CONCEPT_HPP
#define FITORIA_WEB_MIDDLEWARE_CONCEPT_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/core/tag_invoke.hpp>

FITORIA_NAMESPACE_BEGIN

namespace web {
namespace new_middleware_ns {

  template <typename Request, typename Response>
  struct new_middleware_t {
    template <typename Factory, typename Next>
      requires is_tag_invocable_v<new_middleware_t, Factory, Next>
    constexpr auto operator()(Factory&& factory, Next&& next) const
        noexcept(is_nothrow_tag_invocable_v<new_middleware_t, Factory, Next>)
    {
      return tag_invoke(
          *this, std::forward<Factory>(factory), std::forward<Next>(next));
    }
  };

}

using new_middleware_ns::new_middleware_t;

template <typename Request, typename Response>
inline constexpr new_middleware_t<Request, Response> new_middleware {};

}

FITORIA_NAMESPACE_END

#endif
