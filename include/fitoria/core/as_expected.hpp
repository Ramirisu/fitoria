//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_CORE_AS_EXPECTED_HPP
#define FITORIA_CORE_AS_EXPECTED_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/core/detail/boost.hpp>

#include <fitoria/core/expected.hpp>
#include <fitoria/core/type_traits.hpp>

FITORIA_NAMESPACE_BEGIN

template <typename CompletionToken>
class as_expected_t {
public:
  as_expected_t() = default;

  template <not_decay_to<as_expected_t> CompletionToken2>
  constexpr as_expected_t(CompletionToken2&& token)
      : token_(std::forward<CompletionToken2>(token))
  {
  }

  CompletionToken token_;
};

template <typename CompletionToken>
inline constexpr as_expected_t<std::decay_t<CompletionToken>>
as_expected(CompletionToken&& completion_token)
{
  return as_expected_t<std::decay_t<CompletionToken>>(
      std::forward<CompletionToken>(completion_token));
}

template <typename Handler>
class as_expected_handler {
public:
  using result_type = void;

  template <typename CompletionToken>
  as_expected_handler(as_expected_t<CompletionToken> e)
      : handler_(std::forward<CompletionToken>(e.token_))
  {
  }

  template <typename RedirectedHandler>
  as_expected_handler(RedirectedHandler&& h)
      : handler_(static_cast<RedirectedHandler&&>(h))
  {
  }

  void operator()(boost::system::error_code ec)
  {
    if (ec) {
      std::forward<Handler>(handler_)(unexpected { ec });
    } else {
      std::forward<Handler>(handler_)(
          expected<void, boost::system::error_code>());
    }
  }

  template <typename Arg>
  void operator()(boost::system::error_code ec, Arg&& arg)
  {
    if (ec) {
      std::forward<Handler>(handler_)(unexpected { ec });
    } else {
      std::forward<Handler>(handler_)(std::forward<Arg>(arg));
    }
  }

  Handler handler_;
};

template <typename Signature>
struct as_expected_signature;

template <typename R>
struct as_expected_signature<R(boost::system::error_code)> {
  using type = R(expected<void, boost::system::error_code>);
};

template <typename R>
struct as_expected_signature<R(boost::system::error_code)&> {
  using type = R(expected<void, boost::system::error_code>) &;
};

template <typename R>
struct as_expected_signature<R(boost::system::error_code) &&> {
  using type = R(expected<void, boost::system::error_code>) &&;
};

template <typename R, typename Arg>
struct as_expected_signature<R(boost::system::error_code, Arg)> {
  using type = R(expected<std::decay_t<Arg>, boost::system::error_code>);
};

template <typename R, typename Arg>
struct as_expected_signature<R(boost::system::error_code, Arg)&> {
  using type = R(expected<std::decay_t<Arg>, boost::system::error_code>) &;
};

template <typename R, typename Arg>
struct as_expected_signature<R(boost::system::error_code, Arg) &&> {
  using type = R(expected<std::decay_t<Arg>, boost::system::error_code>) &&;
};

#if defined(BOOST_ASIO_HAS_NOEXCEPT_FUNCTION_TYPE)

template <typename R>
struct as_expected_signature<R(boost::system::error_code) noexcept> {
  using type = R(expected<void, boost::system::error_code>) noexcept;
};

template <typename R>
struct as_expected_signature<R(boost::system::error_code) & noexcept> {
  using type = R(expected<void, boost::system::error_code>) & noexcept;
};

template <typename R>
struct as_expected_signature<R(boost::system::error_code) && noexcept> {
  using type = R(expected<void, boost::system::error_code>) && noexcept;
};

template <typename R, typename Arg>
struct as_expected_signature<R(boost::system::error_code, Arg) noexcept> {
  using type
      = R(expected<std::decay_t<Arg>, boost::system::error_code>) noexcept;
};

template <typename R, typename Arg>
struct as_expected_signature<R(boost::system::error_code, Arg) & noexcept> {
  using type
      = R(expected<std::decay_t<Arg>, boost::system::error_code>) & noexcept;
};

template <typename R, typename Arg>
struct as_expected_signature<R(boost::system::error_code, Arg) && noexcept> {
  using type
      = R(expected<std::decay_t<Arg>, boost::system::error_code>) && noexcept;
};

#endif

FITORIA_NAMESPACE_END

namespace boost::asio {

template <typename CompletionToken, typename... Signatures>
struct async_result<fitoria::as_expected_t<CompletionToken>, Signatures...>
    : async_result<
          CompletionToken,
          typename fitoria::as_expected_signature<Signatures>::type...> {
  template <typename Initiation>
  struct init_wrapper {
    init_wrapper(Initiation init)
        : initiation_(std::forward<Initiation>(init))
    {
    }

    template <typename Handler, typename... Args>
    void operator()(Handler&& handler, Args&&... args)
    {
      std::forward<Initiation>(initiation_)(
          fitoria::as_expected_handler<std::decay_t<Handler>>(
              std::forward<Handler>(handler)),
          std::forward<Args>(args)...);
    }

    Initiation initiation_;
  };

  template <typename Initiation, typename RawCompletionToken, typename... Args>
  static auto
  initiate(Initiation&& initiation, RawCompletionToken&& token, Args&&... args)
      -> decltype(async_initiate<
                  std::conditional_t<std::is_const_v<std::remove_reference_t<
                                         RawCompletionToken>>,
                                     const CompletionToken,
                                     CompletionToken>,
                  typename fitoria::as_expected_signature<Signatures>::type...>(
          init_wrapper<std::decay_t<Initiation>>(
              std::forward<Initiation>(initiation)),
          token.token_,
          std::forward<Args>(args)...))
  {
    return async_initiate<
        std::conditional_t<
            std::is_const_v<std::remove_reference_t<RawCompletionToken>>,
            const CompletionToken,
            CompletionToken>,
        typename fitoria::as_expected_signature<Signatures>::type...>(
        init_wrapper<std::decay_t<Initiation>>(
            std::forward<Initiation>(initiation)),
        token.token_,
        std::forward<Args>(args)...);
  }
};

template <template <typename, typename> class Associator,
          typename Handler,
          typename DefaultCandidate>
struct associator<Associator,
                  fitoria::as_expected_handler<Handler>,
                  DefaultCandidate> : Associator<Handler, DefaultCandidate> {
  static typename Associator<Handler, DefaultCandidate>::type
  get(const fitoria::as_expected_handler<Handler>& h) noexcept
  {
    return Associator<Handler, DefaultCandidate>::get(h.handler_);
  }

  static auto get(const fitoria::as_expected_handler<Handler>& h,
                  const DefaultCandidate& c) noexcept
      -> decltype(Associator<Handler, DefaultCandidate>::get(h.handler_, c))
  {
    return Associator<Handler, DefaultCandidate>::get(h.handler_, c);
  }
};

}

#endif
