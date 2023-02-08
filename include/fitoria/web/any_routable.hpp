//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef FITORIA_WEB_ANY_ROUTABLE_HPP
#define FITORIA_WEB_ANY_ROUTABLE_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/core/type_traits.hpp>

#include <fitoria/web/http/http.hpp>
#include <fitoria/web/match_pattern.hpp>
#include <fitoria/web/state_map.hpp>

FITORIA_NAMESPACE_BEGIN

namespace web {

template <typename Request, typename Response>
class any_routable {
  class base {
  public:
    virtual ~base() = default;
    virtual auto operator()(Request) const -> Response = 0;
  };

  template <typename F>
  class derived : public base {
    F f_;

  public:
    derived(F f)
        : f_(std::move(f))
    {
    }

    ~derived() override = default;

    auto operator()(Request req) const -> Response override
    {
      return f_(req);
    }
  };

  http::verb method_;
  match_pattern pattern_;
  std::vector<state_map> state_maps_;
  std::shared_ptr<base> erased_;

public:
  template <typename F>
  any_routable(http::verb method,
               match_pattern pattern,
               std::vector<state_map> state_maps,
               F f)
    requires(!is_specialization_of_v<F, any_routable>)
      : method_(method)
      , pattern_(std::move(pattern))
      , state_maps_(std::move(state_maps))
      , erased_(std::make_shared<derived<F>>(std::move(f)))
  {
  }

  auto method() const noexcept
  {
    return method_;
  }

  auto pattern() const noexcept -> const match_pattern&
  {
    return pattern_;
  }

  auto state_maps() const noexcept -> const std::vector<state_map>&
  {
    return state_maps_;
  }

  auto operator()(Request req) const -> Response
  {
    return erased_->operator()(req);
  }
};
}

FITORIA_NAMESPACE_END

#endif
