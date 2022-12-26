//
// Copyright (c) 2022 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria/http_server/http_server.hpp>

using namespace fitoria;

int main()
{
  auto server = http_server(http_server_config().route(
      router(verb::get, "/api/v1/{owner}/{repo}",
             [](http_context& c) -> net::awaitable<void> {
               FITORIA_ASSERT(c.route().path() == "/api/v1/{owner}/{repo}");
               FITORIA_ASSERT(c.request().method() == verb::get);
               co_return;
             })));
  server
      // start to listen to port 8080
      .bind("127.0.0.1", 8080)
#if defined(FITORIA_HAS_OPENSSL)
      // start to listen to port 443 with SSL enabled
      .bind_ssl("127.0.0.1", 443,
                []() {
                  using net::ssl::context;
                  auto ctx = context(context::tls_server);
                  ctx.set_options(context::default_workarounds
                                  | context::no_sslv2 | context::no_sslv3);
                  return ctx;
                }())
#endif
      // notify workers to start the IO loop
      // notice that `run()` will not block current thread
      .run();

  // register signals to terminate the server
  net::signal_set signal(server.get_execution_context(), SIGINT, SIGTERM);
  signal.async_wait([&](auto, auto) { server.stop(); });

  // block current thread, current thread will process the IO loop
  server.wait();
}
