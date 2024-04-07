//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_CORE_NET_HPP
#define FITORIA_CORE_NET_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/core/detail/boost.hpp>

#include <fitoria/core/type_traits.hpp>

FITORIA_NAMESPACE_BEGIN

#if defined(FITORIA_USE_IO_CONTEXT_EXECUTOR)
using executor_type = boost::asio::io_context::executor_type;
#elif defined(FITORIA_USE_CUSTOM_EXECUTOR)
using executor_type = FITORIA_USE_CUSTOM_EXECUTOR;
#else
using executor_type = boost::asio::any_io_executor;
#endif

template <typename T>
using awaitable = boost::asio::awaitable<T, executor_type>;

constexpr auto use_awaitable
    = boost::asio::as_tuple_t<boost::asio::use_awaitable_t<executor_type>> {};

using socket_acceptor
    = boost::asio::basic_socket_acceptor<boost::asio::ip::tcp, executor_type>;

using tcp_stream
    = boost::beast::basic_stream<boost::asio::ip::tcp, executor_type>;

#if defined(FITORIA_HAS_OPENSSL)

using ssl_stream = boost::beast::ssl_stream<tcp_stream>;

class safe_ssl_stream : public ssl_stream {
public:
  template <typename Arg>
  safe_ssl_stream(Arg&& arg, std::shared_ptr<boost::asio::ssl::context> ssl_ctx)
      : ssl_stream(std::forward<Arg>(arg), *ssl_ctx)
      , ssl_ctx_(std::move(ssl_ctx))
  {
  }

private:
  std::shared_ptr<boost::asio::ssl::context> ssl_ctx_;
};

#endif

namespace net = boost::asio;

FITORIA_NAMESPACE_END

#endif
