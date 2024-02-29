//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
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
#include <fitoria/web/pattern_matcher.hpp>
#include <fitoria/web/state_map.hpp>

#include <functional>

FITORIA_NAMESPACE_BEGIN

namespace web {

template <typename Request, typename Response>
class any_routable {
  class base {
  public:
    virtual ~base() = default;
    virtual auto method() const noexcept -> http::verb = 0;
    virtual auto pattern() const noexcept -> std::string_view = 0;
    virtual auto segments() const noexcept -> const segments_t& = 0;
    virtual auto match(std::string_view) const noexcept -> optional<query_map>
        = 0;
    virtual auto state_maps() const noexcept
        -> const std::vector<state_map>& = 0;
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

    auto pattern() const noexcept -> std::string_view override
    {
      return routable_.matcher().pattern();
    }

    auto segments() const noexcept -> const segments_t& override
    {
      return routable_.matcher().segments();
    }

    auto match(std::string_view path) const noexcept
        -> optional<query_map> override
    {
      return routable_.matcher().match(path);
    }

    auto state_maps() const noexcept -> const std::vector<state_map>& override
    {
      return routable_.state_maps();
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
      : routable_(std::make_shared<derived<std::decay_t<Routable>>>(
          std::forward<Routable>(routable)))
  {
  }

  any_routable(const any_routable&) = default;

  any_routable& operator=(const any_routable&) = default;

  any_routable(any_routable&&) = default;

  any_routable& operator=(any_routable&&) = default;

  auto method() const noexcept -> http::verb
  {
    return routable_->method();
  }

  auto pattern() const noexcept -> std::string_view
  {
    return routable_->pattern();
  }

  auto segments() const noexcept -> const segments_t&
  {
    return routable_->segments();
  }

  auto match(std::string_view path) const noexcept -> optional<query_map>
  {
    return routable_->match(path);
  }

  auto state_maps() const noexcept -> const std::vector<state_map>&
  {
    return routable_->state_maps();
  }

  auto operator()(Request req) const -> Response
  {
    return (*routable_)(req);
  }

private:
  std::shared_ptr<base> routable_;
};

}

FITORIA_NAMESPACE_END

#endif
