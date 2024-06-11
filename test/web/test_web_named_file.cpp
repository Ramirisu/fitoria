//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria/test/test.hpp>

#if defined(FITORIA_HAS_LIBURING)
#define BOOST_ASIO_HAS_IO_URING
#endif

#include <fitoria/test/http_server_utils.hpp>
#include <fitoria/test/utility.hpp>

#include <fitoria/web/named_file.hpp>

#include <fstream>

using namespace fitoria;
using namespace fitoria::web;
using namespace fitoria::test;

TEST_SUITE_BEGIN("[fitoria.web.named_file]");

#if defined(BOOST_ASIO_HAS_FILE)

namespace {

auto create_and_open_tmp_file(std::string_view ext)
    -> awaitable<expected<named_file, std::error_code>>
{
  const auto file_path = get_random_temp_file_path(ext);
  {
    auto ofs = std::ofstream(file_path);
    ofs << file_path;
  }
  co_return named_file::open_readonly(co_await net::this_coro::executor,
                                      file_path);
}
}

TEST_CASE("content_type")
{
  sync_wait([&]() -> awaitable<void> {
    {
      auto file = co_await create_and_open_tmp_file("js");
      CHECK_EQ(file->content_type(), mime::application_javascript());
      file->set_content_type(mime::application_text());
      CHECK_EQ(file->content_type(), mime::application_text());
    }
    {
      auto file = co_await create_and_open_tmp_file("png");
      CHECK_EQ(file->content_type(), mime::image_png());
      file->set_content_type(mime::application_text());
      CHECK_EQ(file->content_type(), mime::application_text());
    }
    {
      auto file = co_await create_and_open_tmp_file("txt");
      CHECK_EQ(file->content_type(), mime::text_plain());
      file->set_content_type(mime::application_text());
      CHECK_EQ(file->content_type(), mime::application_text());
    }
    {
      auto file = co_await create_and_open_tmp_file("tmp");
      CHECK_EQ(file->content_type(), mime::application_octet_stream());
      file->set_content_type(mime::application_text());
      CHECK_EQ(file->content_type(), mime::application_text());
    }
  });
}

TEST_CASE("open_readonly")
{
  sync_wait([]() -> awaitable<void> {
    {
      const auto file_path = get_random_temp_file_path();
      CHECK(!named_file::open_readonly(co_await net::this_coro::executor,
                                       file_path));
    }

    CHECK(co_await create_and_open_tmp_file("tmp"));
  });
}

#endif

TEST_SUITE_END();
