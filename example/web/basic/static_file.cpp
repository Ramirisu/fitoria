//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#if defined(FITORIA_TARGET_LINUX)
#define BOOST_ASIO_HAS_IO_URING
#endif
#include <fitoria/web.hpp>

#if defined(BOOST_ASIO_HAS_FILE)

using namespace fitoria;
using namespace fitoria::web;

auto serve_file(path_of<std::tuple<std::string>> path, const request& req)
    -> awaitable<expected<static_file, response>>
{
  auto [file_path] = path;

  co_return static_file::open(co_await net::this_coro::executor, file_path, req)
      .transform_error([&file_path](auto) -> response {
        return response::not_found()
            .set_header(http::field::content_type, mime::text_plain())
            .set_body(
                fmt::format("requsted file was not found: \"{}\"", file_path));
      });
}

int main()
{
  auto ioc = net::io_context();
  auto server = http_server::builder(ioc)
                    .serve(route::get<"/#file_path">(serve_file))
                    .build();
  server.bind("127.0.0.1", 8080);

  ioc.run();
}

#else

int main() { }

#endif
