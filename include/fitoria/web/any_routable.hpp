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

#include <functional>

FITORIA_NAMESPACE_BEGIN

namespace web {

template <typename Request, typename Response>
class any_routable {
  http::verb method_;
  match_pattern pattern_;
  std::vector<state_map> state_maps_;
  std::function<Response(Request)> service_;

public:
  template <typename Service>
  any_routable(http::verb method,
               match_pattern pattern,
               std::vector<state_map> state_maps,
               Service&& service)
    requires(!is_specialization_of_v<std::remove_cvref_t<Service>,
                                     any_routable>)
      : method_(method)
      , pattern_(std::move(pattern))
      , state_maps_(std::move(state_maps))
      , service_(std::forward<Service>(service))
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
    return service_(req);
  }
};

}

FITORIA_NAMESPACE_END

#endif
