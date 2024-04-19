//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_WEB_ROUTE_HPP
#define FITORIA_WEB_ROUTE_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/core/fixed_string.hpp>
#include <fitoria/core/utility.hpp>

#include <fitoria/web/http/http.hpp>
#include <fitoria/web/path_matcher.hpp>
#include <fitoria/web/path_parser.hpp>
#include <fitoria/web/routable.hpp>
#include <fitoria/web/state_storage.hpp>
#include <fitoria/web/to_middleware.hpp>

#include <tuple>

FITORIA_NAMESPACE_BEGIN

namespace web {

template <basic_fixed_string Path, typename Middlewares, typename Handler>
class route_impl;

template <basic_fixed_string Path, typename... Middlewares, typename Handler>
class route_impl<Path, std::tuple<Middlewares...>, Handler> {
  http::verb method_;
  state_storage states_;
  std::tuple<Middlewares...> middlewares_;
  Handler handler_;

public:
  static_assert(path_parser<true>().parse<Path>(), "invalid path for route");

  route_impl(http::verb method,
             state_storage states,
             std::tuple<Middlewares...> middlewares,
             Handler handler)
      : method_(method)
      , states_(std::move(states))
      , middlewares_(std::move(middlewares))
      , handler_(std::move(handler))
  {
  }

  template <typename State>
  auto use_state(State&& state) const
  {
    using state_type = std::decay_t<State>;
    static_assert(std::copy_constructible<state_type>);

    return route_impl<Path, std::tuple<Middlewares...>, Handler>(
        method_,
        states_.copy_insert_front(std::type_index(typeid(state_type)),
                                  std::any(std::forward<State>(state))),
        middlewares_,
        handler_);
  }

  template <typename Middleware>
  auto use(Middleware&& mw) const
  {
    return route_impl<Path,
                      std::tuple<Middlewares..., std::decay_t<Middleware>>,
                      Handler>(
        method_,
        states_,
        std::tuple_cat(middlewares_,
                       std::tuple { std::forward<Middleware>(mw) }),
        handler_);
  }

  template <basic_fixed_string ParentPath, typename... ParentServices>
  auto rebind_parent(shared_state_map parent_state_map,
                     std::tuple<ParentServices...> parent_services) const
  {
    return route_impl<ParentPath + Path,
                      std::tuple<ParentServices..., Middlewares...>,
                      Handler>(
        method_,
        parent_state_map->empty()
            ? states_
            : states_.copy_append(std::move(parent_state_map)),
        std::tuple_cat(std::move(parent_services), middlewares_),
        handler_);
  }

  template <typename Request, typename Response, typename... AdditionalServices>
  auto build(AdditionalServices... additionalServices) const
  {
    return routable(
        method_,
        path_matcher(Path),
        states_,
        std::apply(
            [](auto... ss) {
              return build_service<Request, Response>(std::move(ss)...);
            },
            reverse_tuple(std::tuple_cat(
                middlewares_,
                std::tuple { std::move(additionalServices)..., handler_ }))));
  }

private:
  template <typename Request, typename Response, typename S0>
  static auto build_service(S0 s0)
  {
    return s0;
  }

  template <typename Request,
            typename Response,
            typename S0,
            typename S1,
            typename... S>
  static auto build_service(S0 s0, S1 s1, S... ss)
  {
    if constexpr (sizeof...(S) > 0) {
      return build_service<Request, Response>(
          to_middleware<Request, Response>(std::move(s1), std::move(s0)),
          std::move(ss)...);
    } else {
      return to_middleware<Request, Response>(std::move(s1), std::move(s0));
    }
  }
};

class route {
public:
  template <basic_fixed_string Path, typename Handler>
  static auto handle(http::verb method, Handler&& handler)
  {
    static_assert(path_parser<true>().parse<Path>(), "invalid path for route");
    return route_impl<Path, std::tuple<>, std::decay_t<Handler>>(
        method, {}, {}, std::forward<Handler>(handler));
  }

  template <basic_fixed_string Path, typename Handler>
  static auto any(Handler&& handler)
  {
    static_assert(path_parser<true>().parse<Path>(), "invalid path for route");
    return route_impl<Path, std::tuple<>, std::decay_t<Handler>>(
        http::verb::unknown, {}, {}, std::forward<Handler>(handler));
  }

  template <basic_fixed_string Path, typename Handler>
  static auto get(Handler&& handler)
  {
    static_assert(path_parser<true>().parse<Path>(), "invalid path for route");
    return handle<Path>(http::verb::get, std::forward<Handler>(handler));
  }

  template <basic_fixed_string Path, typename Handler>
  static auto post(Handler&& handler)
  {
    static_assert(path_parser<true>().parse<Path>(), "invalid path for route");
    return handle<Path>(http::verb::post, std::forward<Handler>(handler));
  }

  template <basic_fixed_string Path, typename Handler>
  static auto put(Handler&& handler)
  {
    static_assert(path_parser<true>().parse<Path>(), "invalid path for route");
    return handle<Path>(http::verb::put, std::forward<Handler>(handler));
  }

  template <basic_fixed_string Path, typename Handler>
  static auto patch(Handler&& handler)
  {
    static_assert(path_parser<true>().parse<Path>(), "invalid path for route");
    return handle<Path>(http::verb::patch, std::forward<Handler>(handler));
  }

  template <basic_fixed_string Path, typename Handler>
  static auto delete_(Handler&& handler)
  {
    static_assert(path_parser<true>().parse<Path>(), "invalid path for route");
    return handle<Path>(http::verb::delete_, std::forward<Handler>(handler));
  }

  template <basic_fixed_string Path, typename Handler>
  static auto head(Handler&& handler)
  {
    static_assert(path_parser<true>().parse<Path>(), "invalid path for route");
    return handle<Path>(http::verb::head, std::forward<Handler>(handler));
  }

  template <basic_fixed_string Path, typename Handler>
  static auto options(Handler&& handler)
  {
    static_assert(path_parser<true>().parse<Path>(), "invalid path for route");
    return handle<Path>(http::verb::options, std::forward<Handler>(handler));
  }
};
}

FITORIA_NAMESPACE_END

#endif
