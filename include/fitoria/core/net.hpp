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

#include <fitoria/core/as_expected.hpp>
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
    = as_expected_t<boost::asio::use_awaitable_t<executor_type>> {};

template <typename Protocol>
using socket_acceptor
    = boost::asio::basic_socket_acceptor<Protocol, executor_type>;

template <typename Protocol>
using basic_stream = boost::beast::basic_stream<Protocol, executor_type>;

#if defined(FITORIA_HAS_OPENSSL)

template <typename Protocol>
using ssl_stream = boost::asio::ssl::stream<basic_stream<Protocol>>;

#endif

using test_stream = boost::beast::test::basic_stream<executor_type>;

template <typename Protocol>
class shared_basic_stream {
  std::shared_ptr<basic_stream<Protocol>> stream_;

public:
  using executor_type = typename basic_stream<Protocol>::executor_type;
  using socket_type = typename basic_stream<Protocol>::socket_type;

  template <not_decay_to<shared_basic_stream> Arg>
  shared_basic_stream(Arg&& arg)
      : stream_(
            std::make_shared<basic_stream<Protocol>>(std::forward<Arg>(arg)))
  {
  }

  shared_basic_stream(const shared_basic_stream&) = default;

  shared_basic_stream& operator=(const shared_basic_stream&) = default;

  shared_basic_stream(shared_basic_stream&&) = default;

  shared_basic_stream& operator=(shared_basic_stream&&) = default;

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

  void expires_never()
  {
    stream_->expires_never();
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

  void shutdown(boost::asio::socket_base::shutdown_type what,
                boost::system::error_code& ec)
  {
    socket().shutdown(what, ec);
  }

  friend void teardown(boost::beast::role_type role,
                       shared_basic_stream& stream,
                       boost::system::error_code& ec)
  {
    using boost::beast::websocket::teardown;
    teardown(role, stream.socket(), ec);
  }

  template <class TeardownHandler>
  friend void async_teardown(boost::beast::role_type role,
                             shared_basic_stream& stream,
                             TeardownHandler&& handler)
  {
    using boost::beast::websocket::async_teardown;
    async_teardown(
        role, stream.socket(), std::forward<TeardownHandler>(handler));
  }

  friend void beast_close_socket(shared_basic_stream& stream)
  {
    boost::system::error_code ec;
    ec = stream.socket().close(ec);
  }
};

class shared_test_stream {
  std::shared_ptr<test_stream> stream_;

public:
  using executor_type = typename test_stream::executor_type;
  using socket_type = test_stream;

  template <not_decay_to<shared_test_stream> Arg>
  shared_test_stream(Arg&& arg)
      : stream_(std::make_shared<test_stream>(std::forward<Arg>(arg)))
  {
  }

  shared_test_stream(const shared_test_stream&) = default;

  shared_test_stream& operator=(const shared_test_stream&) = default;

  shared_test_stream(shared_test_stream&&) = default;

  shared_test_stream& operator=(shared_test_stream&&) = default;

  auto get_executor() -> executor_type
  {
    return stream_->get_executor();
  }

  auto socket() -> socket_type&
  {
    return *stream_;
  }

  void expires_after(boost::asio::steady_timer::duration) { }

  void expires_never() { }

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

  void shutdown(boost::asio::socket_base::shutdown_type,
                boost::system::error_code& ec)
  {
    ec = {};
    stream_->close();
  }

  friend auto connect(shared_test_stream& local) -> shared_test_stream
  {
    auto remote = shared_test_stream(local.get_executor());
    connect(*local.stream_, *remote.stream_);
    return remote;
  }

  friend void teardown(boost::beast::role_type,
                       shared_test_stream& stream,
                       boost::system::error_code&)
  {
    stream.stream_->close();
  }

  template <class TeardownHandler>
  friend void async_teardown(boost::beast::role_type role,
                             shared_test_stream& stream,
                             TeardownHandler&& handler)
  {
    boost::system::error_code ec;
    teardown(role, stream, ec);
    handler(ec);
  }

  friend void beast_close_socket(shared_test_stream& stream)
  {
    stream.stream_->close();
  }
};

#if defined(FITORIA_HAS_OPENSSL)

template <typename Protocol>
class shared_ssl_stream {
  std::shared_ptr<ssl_stream<Protocol>> stream_;

public:
  using executor_type = typename ssl_stream<Protocol>::executor_type;
  using next_layer_type = typename ssl_stream<Protocol>::next_layer_type;

  template <typename Arg>
  shared_ssl_stream(Arg&& arg, boost::asio::ssl::context& ssl_ctx)
      : stream_(std::make_shared<ssl_stream<Protocol>>(std::forward<Arg>(arg),
                                                       ssl_ctx))
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

template <typename NextLayer>
using websocket_stream = boost::beast::websocket::stream<NextLayer>;

#if defined(BOOST_ASIO_HAS_FILE)
using stream_file = boost::asio::basic_stream_file<executor_type>;
#endif

using boost::beast::flat_buffer;
using boost::beast::get_lowest_layer;

namespace net = boost::asio;

FITORIA_NAMESPACE_END

#endif
