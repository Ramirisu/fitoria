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
#include <fitoria/web/pattern_matcher.hpp>
#include <fitoria/web/state_map.hpp>

#include <functional>

FITORIA_NAMESPACE_BEGIN

namespace web {

template <typename Request, typename Response>
class any_routable {
  http::verb method_;
  pattern_matcher matcher_;
  std::vector<state_map> state_maps_;
  std::function<Response(Request)> service_;

public:
  template <typename Service>
  any_routable(http::verb method,
               pattern_matcher matcher,
               std::vector<state_map> state_maps,
               Service&& service)
    requires(!is_specialization_of_v<std::remove_cvref_t<Service>,
                                     any_routable>)
      : method_(method)
      , matcher_(std::move(matcher))
      , state_maps_(std::move(state_maps))
      , service_(std::forward<Service>(service))
  {
  }

  auto method() const noexcept -> http::verb
  {
    return method_;
  }

  auto matcher() const noexcept -> const pattern_matcher&
  {
    return matcher_;
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
