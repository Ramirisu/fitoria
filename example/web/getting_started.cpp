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
  auto server
      = http_server_builder(ioc)
            .serve(route::get<"/echo">(
                [](std::string body) -> net::awaitable<http_response> {
                  co_return http_response(http::status::ok)
                      .set_field(http::field::content_type,
                                 http::fields::content_type::plaintext())
                      .set_body(body);
                }))
            .build();

  server
      .bind("127.0.0.1", 8080)
#if defined(FITORIA_HAS_OPENSSL)
      .bind_ssl("127.0.0.1",
                8443,
                cert::get_server_ssl_ctx(net::ssl::context::method::tls_server))
#endif
      ;

  ioc.run();
}
