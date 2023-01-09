//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria/http_server.hpp>

#include <fitoria_certificate.h>

using namespace fitoria;

// $ ./quick_start
// $ curl -X GET http://127.0.0.1:8080/api/v1/ramirisu/fitoria --verbose
//
// clang-format off
// server output:
// > 2023-01-01T00:00:00Z DEBUG route: /api/v1/{owner}/{repo} [quick_start.cpp:36:26]
// > 2023-01-01T00:00:00Z DEBUG owner: ramirisu, repo: fitoria [quick_start.cpp:37:26]
// clang-format on
//
// $ curl -X GET https://127.0.0.1:8443/api/v1/ramirisu/fitoria --verbose
// --insecure
//
// clang-format off
// server output:
// > 2023-01-01T00:00:00Z DEBUG route: /api/v1/{owner}/{repo} [quick_start.cpp:36:26]
// > 2023-01-01T00:00:00Z DEBUG owner: ramirisu, repo: fitoria [quick_start.cpp:37:26]
// clang-format on

int main()
{
  log::global_logger()->set_log_level(log::level::debug);

  auto server = http_server(http_server_config().route(
      router(http::verb::get, "/api/v1/{owner}/{repo}",
             [](http_request& req) -> net::awaitable<http_response> {
               log::debug("route: {}", req.route().path());
               log::debug("owner: {}, repo: {}", req.route().get("owner"),
                          req.route().get("repo"));

               co_return http_response(http::status::ok)
                   .set_header(http::field::content_type, "text/plain")
                   .set_body("quick start");
             })));
  server
      // Start to listen to port 8080
      .bind("127.0.0.1", 8080)
#if defined(FITORIA_HAS_OPENSSL)
      // Start to listen to port 8443 with SSL enabled
      .bind_ssl("127.0.0.1", 8443,
                cert::get_server_ssl_ctx(net::ssl::context::method::tls_server))
#endif
      // Notify workers to start the IO loop
      // Notice that `run()` will not block current thread
      .run();

  // Register signals to terminate the server
  net::signal_set signal(server.get_execution_context(), SIGINT, SIGTERM);
  signal.async_wait([&](auto, auto) { server.stop(); });

  // Block current thread, current thread will process the IO loop
  server.wait();

  return 0;
}
