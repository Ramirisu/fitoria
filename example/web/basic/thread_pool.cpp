//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria/web.hpp>

using namespace fitoria;
using namespace fitoria::web;

int main()
{
  auto tp = net::static_thread_pool();
  auto server
      = http_server::builder(tp.get_executor())
            .serve(
                route::post<"/">([](std::string body) -> awaitable<response> {
                  co_return response::ok()
                      .set_header(http::field::content_type, mime::text_plain())
                      .set_body(body);
                }))
            .build();
  server.bind("127.0.0.1", 8080);

  tp.join();
}
