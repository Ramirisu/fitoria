//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#if defined(_WIN32)
#include <Windows.h>
#endif

#include <fitoria_http_server_utils.h>

#include <fitoria/fitoria.hpp>

#include <thread>

using namespace fitoria;
using namespace fitoria::web;
using namespace fitoria::http_server_utils;

int main()
{
  auto server = http_server::builder().build();
  net::io_context ioc;
  net::co_spawn(
      ioc, [&]() -> net::awaitable<void> { co_await server.async_run(); },
      net::detached);
  net::thread_pool tp(1);
  net::post(tp, [&]() { ioc.run(); });
  std::this_thread::sleep_for(server_start_wait_time);
  ioc.stop();
}
