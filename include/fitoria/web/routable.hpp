//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_WEB_ROUTABLE_HPP
#define FITORIA_WEB_ROUTABLE_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/http/verb.hpp>

#include <fitoria/web/path_matcher.hpp>
#include <fitoria/web/state_storage.hpp>

FITORIA_NAMESPACE_BEGIN

namespace web {

template <typename Service>
class routable {
public:
  template <typename Service2>
  routable(http::verb method,
           path_matcher matcher,
           state_storage states,
           Service2&& service)
      : method_(method)
      , matcher_(std::move(matcher))
      , states_(std::move(states))
      , service_(std::forward<Service2>(service))
  {
  }

  auto method() const noexcept -> http::verb
  {
    return method_;
  }

  auto matcher() const noexcept -> const path_matcher&
  {
    return matcher_;
  }

  auto states() const noexcept -> const state_storage&
  {
    return states_;
  }

  auto service() const noexcept -> const Service&
  {
    return service_;
  }

private:
  http::verb method_;
  path_matcher matcher_;
  state_storage states_;
  Service service_;
};

template <typename Service>
routable(http::verb, path_matcher, state_storage, Service&&)
    -> routable<std::decay_t<Service>>;
}

FITORIA_NAMESPACE_END

#endif
