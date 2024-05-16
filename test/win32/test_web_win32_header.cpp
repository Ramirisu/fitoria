//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria/core/config.hpp>

#if defined(FITORIA_TARGET_WINDOWS)
#include <Windows.h>
#endif

#include <fitoria/fitoria.hpp>
#include <fitoria/test/http_server_utils.hpp>

#include <thread>

using namespace fitoria;
using namespace fitoria::web;
using namespace fitoria::test;

int main()
{
  net::io_context ioc;
  auto server = http_server::builder(ioc).build();
  server.bind("127.0.0.1", 8080);

  net::thread_pool tp(1);
  net::post(tp, [&]() { ioc.run(); });

  std::this_thread::sleep_for(server_start_wait_time);
  ioc.stop();
}
