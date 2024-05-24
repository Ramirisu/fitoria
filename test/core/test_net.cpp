//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria/test/test.hpp>

#include <fitoria/core/net.hpp>

using namespace fitoria;

TEST_SUITE_BEGIN("[fitoria.core.net]");

TEST_CASE("executor_type")
{
  static_assert(std::same_as<executor_type, net::any_io_executor>);
  static_assert(
      std::same_as<awaitable<int>, net::awaitable<int, net::any_io_executor>>);
  static_assert(
      std::same_as<
          socket_acceptor<net::ip::tcp>,
          net::basic_socket_acceptor<net::ip::tcp, net::any_io_executor>>);
  static_assert(
      std::same_as<
          basic_stream<net::ip::tcp>,
          boost::beast::basic_stream<net::ip::tcp, net::any_io_executor>>);
}

TEST_SUITE_END();
