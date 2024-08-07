//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#define FITORIA_USE_IO_CONTEXT_EXECUTOR
#include <fitoria/test/test.hpp>

#include <fitoria/core/net.hpp>

using namespace fitoria;

TEST_SUITE_BEGIN("[fitoria.core.net.io_context]");

TEST_CASE("executor_type")
{

  static_assert(std::same_as<executor_type, net::io_context::executor_type>);
  static_assert(
      std::same_as<awaitable<int>,
                   net::awaitable<int, net::io_context::executor_type>>);
  static_assert(
      std::same_as<socket_acceptor<net::ip::tcp>,
                   net::basic_socket_acceptor<net::ip::tcp,
                                              net::io_context::executor_type>>);
  static_assert(
      std::same_as<basic_stream<net::ip::tcp>,
                   boost::beast::basic_stream<net::ip::tcp,
                                              net::io_context::executor_type>>);
  static_assert(
      std::same_as<basic_stream<net::local::stream_protocol>,
                   boost::beast::basic_stream<net::local::stream_protocol,
                                              net::io_context::executor_type>>);
#if defined(FITORIA_HAS_OPENSSL)
  static_assert(
      std::same_as<ssl_stream<net::ip::tcp>,
                   boost::asio::ssl::stream<boost::beast::basic_stream<
                       net::ip::tcp,
                       net::io_context::executor_type>>>);
  static_assert(
      std::same_as<ssl_stream<net::local::stream_protocol>,
                   boost::asio::ssl::stream<boost::beast::basic_stream<
                       net::local::stream_protocol,
                       net::io_context::executor_type>>>);
#endif
  static_assert(
      std::same_as<
          test_stream,
          boost::beast::test::basic_stream<net::io_context::executor_type>>);
}

TEST_SUITE_END();
