//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_WEB_ANY_ROUTABLE_HPP
#define FITORIA_WEB_ANY_ROUTABLE_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/core/type_traits.hpp>

#include <fitoria/web/http/http.hpp>
#include <fitoria/web/path_matcher.hpp>
#include <fitoria/web/state_storage.hpp>

FITORIA_NAMESPACE_BEGIN

namespace web {

template <typename Request, typename Response>
class any_routable {
  class base {
  public:
    virtual ~base() = default;
    virtual auto method() const noexcept -> http::verb = 0;
    virtual auto matcher() const noexcept -> const path_matcher& = 0;
    virtual auto states() const noexcept -> const state_storage& = 0;
    virtual auto operator()(Request) const -> Response = 0;
  };

  template <typename Routable>
  class derived : public base {
  public:
    derived(Routable routable)
        : routable_(std::move(routable))
    {
    }

    ~derived() override = default;

    auto method() const noexcept -> http::verb override
    {
      return routable_.method();
    }

    auto matcher() const noexcept -> const path_matcher& override
    {
      return routable_.matcher();
    }

    auto states() const noexcept -> const state_storage& override
    {
      return routable_.states();
    }

    auto operator()(Request req) const -> Response override
    {
      return routable_.service()(req);
    }

  private:
    Routable routable_;
  };

public:
  template <typename Routable>
  explicit any_routable(Routable&& routable)
    requires(
        !is_specialization_of_v<std::remove_cvref_t<Routable>, any_routable>)
      : routable_(std::make_unique<derived<std::decay_t<Routable>>>(
            std::forward<Routable>(routable)))
  {
  }

  any_routable(const any_routable&) = delete;

  any_routable& operator=(const any_routable&) = delete;

  any_routable(any_routable&&) = default;

  any_routable& operator=(any_routable&&) = default;

  auto method() const noexcept -> http::verb
  {
    return routable_->method();
  }

  auto matcher() const noexcept -> const path_matcher&
  {
    return routable_->matcher();
  }

  auto states() const noexcept -> const state_storage&
  {
    return routable_->states();
  }

  auto operator()(Request req) const -> Response
  {
    return (*routable_)(req);
  }

private:
  std::unique_ptr<base> routable_;
};

}

FITORIA_NAMESPACE_END

#endif
