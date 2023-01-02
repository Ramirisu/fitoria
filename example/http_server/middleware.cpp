//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria/http_server.hpp>

#include <iostream>

using namespace fitoria;

int main()
{
  auto server = http_server(http_server_config().route(
      // add a router group
      router_group("/api/v1")
          // register a middleware for this group
          .use([](http_context& c)
                   -> net::awaitable<http_response> {
            std::cout << "before handler\n";

            auto res = co_await c.next();

            std::cout << "after handler\n";

            co_return res;
          })
          // register a route for this group
          // the route is associated with the middleware defined above
          .route(router(
              http::verb::get, "/users/{user}",
              [](http_request& req)
                  -> net::awaitable<http_response> {
                FITORIA_ASSERT(req.method() == http::verb::get);

                std::cout << req.route().at("user") << "\n";

                co_return http_response(http::status::ok);
              }))));
  server //
      .bind("127.0.0.1", 8080)
      .run();

  server.wait();

  // $ curl -X GET http://127.0.0.1:8080/api/v1/users/ramirisu
  //
  // before handler
  // ramirisu
  // after handler
}
