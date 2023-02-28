//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria/fitoria.hpp>

#include <fitoria_certificate.h>

using namespace fitoria;
using namespace fitoria::web;

// clang-format off
// 
// $ ./quick_start
// $ curl -X GET http://127.0.0.1:8080/api/v1/ramirisu/fitoria -v
//
// server output:
// > 2023-01-01T00:00:00Z DEBUG route: /api/v1/{owner}/{repo} [quick_start.cpp:36:26]
// > 2023-01-01T00:00:00Z DEBUG owner: ramirisu, repo: fitoria [quick_start.cpp:37:26]
//
// $ curl -X GET https://127.0.0.1:8443/api/v1/ramirisu/fitoria -v --insecure
//
// server output:
// > 2023-01-01T00:00:00Z DEBUG route: /api/v1/{owner}/{repo} [quick_start.cpp:36:26]
// > 2023-01-01T00:00:00Z DEBUG owner: ramirisu, repo: fitoria [quick_start.cpp:37:26]
// 
//
// clang-format on

int main()
{
  log::global_logger() = log::stdout_logger();
  log::global_logger()->set_log_level(log::level::debug);

  auto server = http_server::builder()
                    .route(route::GET<"/api/v1/{owner}/{repo}">(
                        [](http_request& req) -> lazy<http_response> {
                          log::debug("route: {}", req.params().path());
                          log::debug("owner: {}, repo: {}",
                                     req.params().get("owner"),
                                     req.params().get("repo"));

                          co_return http_response(http::status::ok)
                              .set_plaintext("getting started");
                        }))
                    .build();
  server
      // Start to listen to port 8080
      .bind("127.0.0.1", 8080)
#if defined(FITORIA_HAS_OPENSSL)
      // Start to listen to port 8443 with SSL enabled
      .bind_ssl("127.0.0.1",
                8443,
                cert::get_server_ssl_ctx(net::ssl::context::method::tls_server))
#endif
      // Start the server, `run()` will block current thread.
      .run();
}
