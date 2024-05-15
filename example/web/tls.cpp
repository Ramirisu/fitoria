//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria/web.hpp>

#include <fitoria_certificate.h>

using namespace fitoria;
using namespace fitoria::web;

int main()
{
  auto ioc = net::io_context();
  auto server = http_server_builder(ioc)
                    .serve(route::get<"/">([]() -> awaitable<response> {
                      co_return response::ok()
                          .set_field(http::field::content_type,
                                     http::fields::content_type::plaintext())
                          .set_body("Hello World!");
                    }))
                    .build();

  server.bind("127.0.0.1", 8080);
#if defined(FITORIA_HAS_OPENSSL)
  auto ssl_ctx = cert::get_server_ssl_ctx(net::ssl::context::tls_server);
  server.bind_ssl("127.0.0.1", 8443, ssl_ctx);
#endif

  ioc.run();
}
