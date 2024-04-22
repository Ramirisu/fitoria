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

#endif

class shared_tcp_stream {
  std::shared_ptr<tcp_stream> stream_;

public:
  using executor_type = typename tcp_stream::executor_type;
  using socket_type = typename tcp_stream::socket_type;

  template <not_decay_to<shared_tcp_stream> Arg>
  shared_tcp_stream(Arg&& arg)
      : stream_(std::make_shared<tcp_stream>(std::forward<Arg>(arg)))
  {
  }

  shared_tcp_stream(const shared_tcp_stream&) = default;

  shared_tcp_stream& operator=(const shared_tcp_stream&) = default;

  shared_tcp_stream(shared_tcp_stream&&) = default;

  shared_tcp_stream& operator=(shared_tcp_stream&&) = default;

  auto get_executor() -> executor_type
  {
    return stream_->get_executor();
  }

  auto socket() -> socket_type&
  {
    return stream_->socket();
  }

  void expires_after(boost::asio::steady_timer::duration expiry_time)
  {
    stream_->expires_after(expiry_time);
  }

  template <typename ConstBufferSequence, typename WriteHandler>
  auto async_write_some(const ConstBufferSequence& buffers,
                        WriteHandler&& handler)
  {
    return stream_->async_write_some(buffers,
                                     std::forward<WriteHandler>(handler));
  }

  template <typename MutableBufferSequence, typename ReadHandler>
  auto async_read_some(const MutableBufferSequence& buffers,
                       ReadHandler&& handler)
  {
    return stream_->async_read_some(buffers,
                                    std::forward<ReadHandler>(handler));
  }

  friend void teardown(boost::beast::role_type role,
                       shared_tcp_stream& stream,
                       boost::system::error_code& ec)
  {
    using boost::beast::websocket::teardown;
    teardown(role, stream.socket(), ec);
  }

  template <class TeardownHandler>
  friend void async_teardown(boost::beast::role_type role,
                             shared_tcp_stream& stream,
                             TeardownHandler&& handler)
  {
    using boost::beast::websocket::async_teardown;
    async_teardown(
        role, stream.socket(), std::forward<TeardownHandler>(handler));
  }

  friend void beast_close_socket(shared_tcp_stream& stream)
  {
    boost::system::error_code ec;
    ec = stream.socket().close(ec);
  }
};

#if defined(FITORIA_HAS_OPENSSL)
class shared_ssl_stream {
  std::shared_ptr<ssl_stream> stream_;

public:
  using executor_type = typename ssl_stream::executor_type;
  using next_layer_type = typename ssl_stream::next_layer_type;

  template <typename Arg>
  shared_ssl_stream(Arg&& arg, boost::asio::ssl::context& ssl_ctx)
      : stream_(std::make_shared<ssl_stream>(std::forward<Arg>(arg), ssl_ctx))
  {
  }

  shared_ssl_stream(const shared_ssl_stream&) = default;

  shared_ssl_stream& operator=(const shared_ssl_stream&) = default;

  shared_ssl_stream(shared_ssl_stream&&) = default;

  shared_ssl_stream& operator=(shared_ssl_stream&&) = default;

  auto get_executor() -> executor_type
  {
    return stream_->get_executor();
  }

  auto next_layer() noexcept -> next_layer_type&
  {
    return stream_->next_layer();
  }

  auto next_layer() const noexcept -> const next_layer_type&
  {
    return stream_->next_layer();
  }

  template <typename HandshakeHandler>
  auto async_handshake(boost::asio::ssl::stream_base::handshake_type type,
                       HandshakeHandler&& handler)
  {
    return stream_->async_handshake(type,
                                    std::forward<HandshakeHandler>(handler));
  }

  template <typename ConstBufferSequence, typename WriteHandler>
  auto async_write_some(const ConstBufferSequence& buffers,
                        WriteHandler&& handler)
  {
    return stream_->async_write_some(buffers,
                                     std::forward<WriteHandler>(handler));
  }

  template <typename MutableBufferSequence, typename ReadHandler>
  auto async_read_some(const MutableBufferSequence& buffers,
                       ReadHandler&& handler)
  {
    return stream_->async_read_some(buffers,
                                    std::forward<ReadHandler>(handler));
  }

  template <class ShutdownHandler>
  auto async_shutdown(ShutdownHandler&& handler)
  {
    return stream_->async_shutdown(std::forward<ShutdownHandler>(handler));
  }

  friend void teardown(boost::beast::role_type role,
                       shared_ssl_stream& stream,
                       boost::system::error_code& ec)
  {
    using boost::beast::websocket::teardown;
    teardown(role, stream.next_layer(), ec);
  }

  template <class TeardownHandler>
  friend void async_teardown(boost::beast::role_type role,
                             shared_ssl_stream& stream,
                             TeardownHandler&& handler)
  {
    using boost::beast::websocket::async_teardown;
    async_teardown(
        role, stream.next_layer(), std::forward<TeardownHandler>(handler));
  }

  friend void beast_close_socket(shared_ssl_stream& stream)
  {
    stream.next_layer().close();
  }
};

#endif

using ws_stream = boost::beast::websocket::stream<shared_tcp_stream>;

#if defined(FITORIA_HAS_OPENSSL)

using wss_stream = boost::beast::websocket::stream<shared_ssl_stream>;

#endif

namespace net = boost::asio;

FITORIA_NAMESPACE_END

#endif
