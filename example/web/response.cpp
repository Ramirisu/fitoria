//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria/web.hpp>

using namespace fitoria;
using namespace fitoria::web;

auto resp() -> awaitable<response>
{
  co_return response::ok()
      .set_header(http::field::content_type,
                  http::fields::content_type::plaintext())
      .set_body("Hello World!");
}

auto text() -> awaitable<std::string>
{
  co_return "Hello World!";
}

auto binary() -> awaitable<std::vector<std::uint8_t>>
{
  co_return std::vector<std::uint8_t> { 0x48, 0x65, 0x6c, 0x6c, 0x6f, 0x20,
                                        0x57, 0x6f, 0x72, 0x6c, 0x64, 0x21 };
}

auto variant(path_of<std::tuple<bool>> path)
    -> awaitable<std::variant<std::string, std::vector<std::uint8_t>>>
{
  auto [text] = path;
  if (text) {
    co_return "Hello World!";
  } else {
    co_return std::vector<std::uint8_t> { 0x48, 0x65, 0x6c, 0x6c, 0x6f, 0x20,
                                          0x57, 0x6f, 0x72, 0x6c, 0x64, 0x21 };
  }
}

class my_error {
  std::string msg_;

public:
  my_error(std::string msg)
      : msg_(std::move(msg))
  {
  }

  auto message() const noexcept -> const std::string&
  {
    return msg_;
  }

  template <decay_to<my_error> Self>
  friend auto tag_invoke(fitoria::web::to_response_t, Self&& self) -> response
  {
    return response::not_found()
        .set_header(http::field::content_type,
                    http::fields::content_type::plaintext())
        .set_body(self.message());
  }
};

auto err() -> awaitable<my_error>
{
  co_return my_error("You will never get anything!");
}

int main()
{
  auto ioc = net::io_context();
  auto server = http_server::builder(ioc)
                    .serve(route::get<"/response">(resp))
                    .serve(route::get<"/text">(text))
                    .serve(route::get<"/binary">(binary))
                    .serve(route::get<"/variant/{text}">(variant))
                    .serve(route::get<"/error">(err))
                    .build();

  server.bind("127.0.0.1", 8080);

  ioc.run();
}
