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
  http_server server;
  server.route(router { methods::get, "/api/v1/:owner/:repo",
                        [](http_context& c) -> net::awaitable<void> {
                          FITORIA_ASSERT(c.path() == "/api/v1/:owner/:repo");
                          FITORIA_ASSERT(c.request().method() == methods::get);
                          co_return;
                        } });
  server.run("127.0.0.1", 8080);
  server.wait();
}
