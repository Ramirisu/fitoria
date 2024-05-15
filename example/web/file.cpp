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

auto get_static_file(const path_info& pi)
    -> awaitable<std::variant<stream_file, response>>
{
  auto path = pi.at("file_path");
  if (auto file = co_await stream_file::async_open_readonly(path); file) {
    co_return std::move(*file);
  }

  co_return response::not_found()
      .set_field(http::field::content_type,
                 http::fields::content_type::plaintext())
      .set_body(fmt::format("requsted file was not found: \"{}\"", path));
}

int main()
{
  auto ioc = net::io_context();
  auto server = http_server_builder(ioc)
                    .serve(route::get<"/static/#file_path">(get_static_file))
                    .build();
  server.bind("127.0.0.1", 8080);

  ioc.run();
}

#else

int main() { }

#endif
