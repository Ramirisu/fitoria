//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria/web.hpp>

#include <fitoria_certificate.h>

#include <filesystem>

using namespace fitoria;
using namespace fitoria::web;

int main()
{
  auto ioc = net::io_context();
  auto server
      = http_server::builder(ioc)
            .serve(
                route::post<"/">([](std::string body) -> awaitable<response> {
                  co_return response::ok()
                      .set_header(http::field::content_type, mime::text_plain())
                      .set_body(body);
                }))
            .build();

  const auto path = std::filesystem::temp_directory_path() / "fitoria";
  std::filesystem::remove(path); // make sure no file exists

  server.bind_local(path.string());

#if defined(FITORIA_HAS_OPENSSL)
  const auto tls_path = std::filesystem::temp_directory_path() / "fitoria.tls";
  std::filesystem::remove(tls_path); // make sure no file exists

  auto ssl_ctx = cert::get_server_ssl_ctx(net::ssl::context::tls_server);
  server.bind_local(path.string(), ssl_ctx);
#endif

  ioc.run();
}
