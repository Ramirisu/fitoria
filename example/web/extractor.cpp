//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria/fitoria.hpp>

using namespace fitoria;

int main()
{
  auto server
      = http_server::builder()
            .route(
                scope("/api/v1")
                    // Extract `http_request`
                    .route(
                        http::verb::get, "/1",
                        [](http_request& req) -> net::awaitable<http_response> {
                          log::debug("path: {}", req.route().path());

                          co_return http_response(http::status::ok)
                              .set_header(http::field::content_type,
                                          "text/plain")
                              .set_body("extractor");
                        })
                    // Extract `http_route`
                    .route(http::verb::get, "/2",
                           [](const http_route& r)
                               -> net::awaitable<http_response> {
                             log::debug("path: {}", r.path());

                             co_return http_response(http::status::ok)
                                 .set_header(http::field::content_type,
                                             "text/plain")
                                 .set_body("extractor");
                           })
                    // Extract `query_map`
                    .route(http::verb::get, "/3",
                           [](const query_map& qm)
                               -> net::awaitable<http_response> {
                             log::debug("brand: {}", qm.get("brand"));

                             co_return http_response(http::status::ok)
                                 .set_header(http::field::content_type,
                                             "text/plain")
                                 .set_body("extractor");
                           })
                    // Extract `http_header`
                    .route(http::verb::get, "/4",
                           [](const http_header& header)
                               -> net::awaitable<http_response> {
                             log::debug("Content-Type: {}",
                                        header.get(http::field::content_type));

                             co_return http_response(http::status::ok)
                                 .set_header(http::field::content_type,
                                             "text/plain")
                                 .set_body("extractor");
                           })
                    // Extract body as `std::string`
                    .route(http::verb::get, "/5",
                           [](const std::string& body)
                               -> net::awaitable<http_response> {
                             log::debug("body: {}", body);

                             co_return http_response(http::status::ok)
                                 .set_header(http::field::content_type,
                                             "text/plain")
                                 .set_body("extractor");
                           })
                    // Extract arbitrary types that can be converted from
                    // `http_request`
                    .route(http::verb::get, "/nations",
                           [](const query_map& qm, const http_header& header,
                              const std::string& body)
                               -> net::awaitable<http_response> {
                             log::debug("brand: {}", qm.get("brand"));
                             log::debug("Content-Type: {}",
                                        header.get(http::field::content_type));
                             log::debug("body: {}", body);

                             co_return http_response(http::status::ok)
                                 .set_header(http::field::content_type,
                                             "text/plain")
                                 .set_body("extractor");
                           }))
            .build();
  server //
      .bind("127.0.0.1", 8080)
      .run();
}
