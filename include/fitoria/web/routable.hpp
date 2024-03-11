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

#include <fitoria/web/http/http.hpp>
#include <fitoria/web/path_matcher.hpp>
#include <fitoria/web/state_map.hpp>

FITORIA_NAMESPACE_BEGIN

namespace web {

template <typename Service>
class routable {
public:
  template <typename Service2>
  routable(http::verb method,
           path_matcher matcher,
           std::vector<shared_state_map> state_maps,
           Service2&& service)
      : method_(method)
      , matcher_(std::move(matcher))
      , state_maps_(std::move(state_maps))
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

  auto state_maps() const noexcept -> const std::vector<shared_state_map>&
  {
    return state_maps_;
  }

  auto service() const noexcept -> const Service&
  {
    return service_;
  }

private:
  http::verb method_;
  path_matcher matcher_;
  std::vector<shared_state_map> state_maps_;
  Service service_;
};

template <typename Service>
routable(http::verb, path_matcher, std::vector<shared_state_map>, Service&&)
    -> routable<std::decay_t<Service>>;
}

FITORIA_NAMESPACE_END

#endif
