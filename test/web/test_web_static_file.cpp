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

#include <fitoria/web.hpp>

#include <fstream>

using namespace fitoria;
using namespace fitoria::web;
using namespace fitoria::test;

TEST_SUITE_BEGIN("[fitoria.web.static_file]");

#if defined(BOOST_ASIO_HAS_FILE)

namespace {
auto get_etag(
    const std::uint64_t size,
    const std::chrono::time_point<std::chrono::file_clock>& last_modified_time)
{
  return http::header::entity_tag::make_strong(
      fmt::format("{}-{:016x}",
                  size,
                  std::chrono::floor<std::chrono::seconds>(
                      chrono::to_utc(last_modified_time))
                      .time_since_epoch()
                      .count()));
}
}

TEST_CASE("etag & content_type & content_disposition")
{
  auto create_and_open_tmp_file = [](const std::string& file_path)
      -> awaitable<expected<static_file, std::error_code>> {
    {
      auto ofs = std::ofstream(file_path);
      ofs << file_path;
    }
    co_return static_file::open(co_await net::this_coro::executor, file_path);
  };

  sync_wait([&]() -> awaitable<void> {
    {
      const auto file_path = get_temp_file_path("js");
      auto file = co_await create_and_open_tmp_file(file_path);
      CHECK(file->file().is_open());
      CHECK_EQ(file->etag(),
               get_etag(file->file().size(),
                        std::filesystem::last_write_time(file_path)));
      CHECK_EQ(file->content_type(), mime::application_javascript());
      CHECK_EQ(file->content_disposition(), "inline");
      file->set_content_type(mime::application_text());
      CHECK_EQ(file->content_type(), mime::application_text());

      const auto& cfile = file;
      CHECK(cfile->file().is_open());
    }
    {
      const auto file_path = get_temp_file_path("png");
      auto file = co_await create_and_open_tmp_file(file_path);
      CHECK(file->file().is_open());
      CHECK_EQ(file->etag(),
               get_etag(file->file().size(),
                        std::filesystem::last_write_time(file_path)));
      CHECK_EQ(file->content_type(), mime::image_png());
      CHECK_EQ(file->content_disposition(), "inline");
      file->set_content_type(mime::application_text());
      CHECK_EQ(file->content_type(), mime::application_text());

      const auto& cfile = file;
      CHECK(cfile->file().is_open());
    }
    {
      const auto file_path = get_temp_file_path("txt");
      auto file = co_await create_and_open_tmp_file(file_path);
      CHECK(file->file().is_open());
      CHECK_EQ(file->etag(),
               get_etag(file->file().size(),
                        std::filesystem::last_write_time(file_path)));
      CHECK_EQ(file->content_type(), mime::text_plain());
      CHECK_EQ(file->content_disposition(), "inline");
      file->set_content_type(mime::application_text());
      CHECK_EQ(file->content_type(), mime::application_text());

      const auto& cfile = file;
      CHECK(cfile->file().is_open());
    }
    {
      const auto file_path = get_temp_file_path("tmp");
      auto file = co_await create_and_open_tmp_file(file_path);
      CHECK(file->file().is_open());
      CHECK_EQ(file->etag(),
               get_etag(file->file().size(),
                        std::filesystem::last_write_time(file_path)));
      CHECK_EQ(file->content_type(), mime::application_octet_stream());
      CHECK_EQ(
          file->content_disposition(),
          fmt::format(R"(attachment; filename="{}")",
                      std::filesystem::path(file_path).filename().string()));
      file->set_content_type(mime::application_text());
      CHECK_EQ(file->content_type(), mime::application_text());

      const auto& cfile = file;
      CHECK(cfile->file().is_open());
    }
  });
}

TEST_CASE("open w/o request header")
{
  const auto file_path = get_temp_file_path();
  const auto data = get_random_string(1048576);
  {
    std::ofstream(file_path, std::ios::binary) << data;
  }
  const auto lmd = std::filesystem::last_write_time(file_path);
  const auto lmd_str = http::header::date(lmd).to_string();
  const auto cd_str
      = fmt::format(R"(attachment; filename="{}")",
                    std::filesystem::path(file_path).filename().string());
  const auto etag_str = get_etag(data.size(), lmd).to_string();

  auto ioc = net::io_context();
  auto server = http_server::builder(ioc)
                    .serve(route::get<"/">(
                        [&file_path]()
                            -> awaitable<std::variant<static_file, response>> {
                          if (auto file = static_file::open(
                                  co_await net::this_coro::executor, file_path);
                              file) {
                            co_return std::move(*file);
                          }

                          co_return response::not_found().build();
                        }))
                    .build();

  server.serve_request(
      "/",
      test_request::get().build(),
      [&](test_response res) -> awaitable<void> {
        CHECK_EQ(res.status(), http::status::ok);
        CHECK_EQ(res.headers().get(http::field::last_modified), lmd_str);
        CHECK_EQ(res.headers().get(http::field::etag), etag_str);
        CHECK_EQ(res.headers().get(http::field::content_type),
                 mime::application_octet_stream());
        CHECK_EQ(res.headers().get(http::field::content_disposition), cd_str);
        CHECK(!res.headers().get(http::field::accept_ranges));
        CHECK(!res.headers().get(http::field::content_range));
        CHECK_EQ(co_await res.as_string(), data);
      });
  server.serve_request(
      "/",
      test_request::get()
          .set_header(http::field::range, "bytes=100000-299999, 300000-399999")
          .build(),
      [&](test_response res) -> awaitable<void> {
        CHECK_EQ(res.status(), http::status::ok);
        CHECK_EQ(res.headers().get(http::field::last_modified), lmd_str);
        CHECK_EQ(res.headers().get(http::field::etag), etag_str);
        CHECK_EQ(res.headers().get(http::field::content_type),
                 mime::application_octet_stream());
        CHECK_EQ(res.headers().get(http::field::content_disposition), cd_str);
        CHECK(!res.headers().get(http::field::accept_ranges));
        CHECK(!res.headers().get(http::field::content_range));
        CHECK_EQ(co_await res.as_string(), data);
      });

  ioc.run();
}

TEST_CASE("open with request header: range")
{
  const auto file_path = get_temp_file_path();
  const auto data = get_random_string(1048576);
  {
    std::ofstream(file_path, std::ios::binary) << data;
  }
  const auto lmd = std::filesystem::last_write_time(file_path);
  const auto lmd_str = http::header::date(lmd).to_string();
  const auto cd_str
      = fmt::format(R"(attachment; filename="{}")",
                    std::filesystem::path(file_path).filename().string());
  const auto etag_str = get_etag(data.size(), lmd).to_string();

  auto ioc = net::io_context();
  auto server
      = http_server::builder(ioc)
            .serve(route::get<"/">(
                [&file_path](const request& req)
                    -> awaitable<std::variant<static_file, response>> {
                  if (auto file = static_file::open(
                          co_await net::this_coro::executor, file_path, req);
                      file) {
                    co_return std::move(*file);
                  }

                  co_return response::not_found().build();
                }))
            .build();

  server.serve_request(
      "/",
      test_request::get().build(),
      [&](test_response res) -> awaitable<void> {
        CHECK_EQ(res.status(), http::status::ok);
        CHECK_EQ(res.headers().get(http::field::last_modified), lmd_str);
        CHECK_EQ(res.headers().get(http::field::etag), etag_str);
        CHECK_EQ(res.headers().get(http::field::content_type),
                 mime::application_octet_stream());
        CHECK_EQ(res.headers().get(http::field::content_disposition), cd_str);
        CHECK_EQ(res.headers().get(http::field::accept_ranges), "bytes");
        CHECK(!res.headers().get(http::field::content_range));
        CHECK_EQ(co_await res.as_string(), data);
        co_return;
      });
  server.serve_request(
      "/",
      test_request::get()
          .set_header(http::field::range, "bytes=100000-299999, 300000-399999")
          .build(),
      [&](test_response res) -> awaitable<void> {
        CHECK_EQ(res.status(), http::status::partial_content);
        CHECK_EQ(res.headers().get(http::field::last_modified), lmd_str);
        CHECK_EQ(res.headers().get(http::field::etag), etag_str);
        CHECK_EQ(res.headers().get(http::field::content_type),
                 mime::application_octet_stream());
        CHECK_EQ(res.headers().get(http::field::content_disposition), cd_str);
        CHECK_EQ(res.headers().get(http::field::accept_ranges), "bytes");
        CHECK_EQ(res.headers().get(http::field::content_range),
                 "bytes 100000-299999/200000");
        CHECK_EQ(co_await res.as_string(),
                 std::string_view(data).substr(100000, 200000));
        co_return;
      });
  server.serve_request(
      "/",
      test_request::get()
          .set_header(http::field::range, "bytes=900000-")
          .build(),
      [&](test_response res) -> awaitable<void> {
        CHECK_EQ(res.status(), http::status::partial_content);
        CHECK_EQ(res.headers().get(http::field::last_modified), lmd_str);
        CHECK_EQ(res.headers().get(http::field::etag), etag_str);
        CHECK_EQ(res.headers().get(http::field::content_type),
                 mime::application_octet_stream());
        CHECK_EQ(res.headers().get(http::field::content_disposition), cd_str);
        CHECK_EQ(res.headers().get(http::field::accept_ranges), "bytes");
        CHECK_EQ(res.headers().get(http::field::content_range),
                 "bytes 900000-1048575/148576");
        CHECK_EQ(co_await res.as_string(),
                 std::string_view(data).substr(900000, 148576));
        co_return;
      });
  server.serve_request(
      "/",
      test_request::get()
          .set_header(http::field::range, "bytes=1000000-1299999")
          .build(),
      [](test_response res) -> awaitable<void> {
        CHECK_EQ(res.status(), http::status::range_not_satisfiable);
        CHECK(!res.headers().get(http::field::last_modified));
        CHECK(!res.headers().get(http::field::etag));
        CHECK_EQ(res.headers().get(http::field::accept_ranges), "bytes");
        CHECK_EQ(res.headers().get(http::field::content_range),
                 "bytes */1048576");
        co_return;
      });
  server.serve_request(
      "/",
      test_request::get()
          .set_header(http::field::range, "x=100000-299999")
          .build(),
      [](test_response res) -> awaitable<void> {
        CHECK_EQ(res.status(), http::status::range_not_satisfiable);
        CHECK(!res.headers().get(http::field::last_modified));
        CHECK(!res.headers().get(http::field::etag));
        CHECK_EQ(res.headers().get(http::field::accept_ranges), "bytes");
        CHECK_EQ(res.headers().get(http::field::content_range),
                 "bytes */1048576");
        co_return;
      });

  ioc.run();
}

TEST_CASE("open with request header: if_none_match")
{
  const auto file_path = get_temp_file_path();
  const auto data = get_random_string(1048576);
  {
    std::ofstream(file_path, std::ios::binary) << data;
  }
  const auto lmd = std::filesystem::last_write_time(file_path);
  const auto lmd_str = http::header::date(lmd).to_string();
  const auto cd_str
      = fmt::format(R"(attachment; filename="{}")",
                    std::filesystem::path(file_path).filename().string());
  const auto etag_str = get_etag(data.size(), lmd).to_string();

  auto ioc = net::io_context();
  auto server
      = http_server::builder(ioc)
            .serve(route::get<"/">(
                [&file_path](const request& req)
                    -> awaitable<std::variant<static_file, response>> {
                  if (auto file = static_file::open(
                          co_await net::this_coro::executor, file_path, req);
                      file) {
                    co_return std::move(*file);
                  }

                  co_return response::not_found().build();
                }))
            .build();

  server.serve_request(
      "/",
      test_request::get().build(),
      [&](test_response res) -> awaitable<void> {
        CHECK_EQ(res.status(), http::status::ok);
        CHECK_EQ(res.headers().get(http::field::last_modified), lmd_str);
        CHECK_EQ(res.headers().get(http::field::etag), etag_str);
        CHECK_EQ(res.headers().get(http::field::content_type),
                 mime::application_octet_stream());
        CHECK_EQ(res.headers().get(http::field::content_disposition), cd_str);
        CHECK_EQ(res.headers().get(http::field::accept_ranges), "bytes");
        CHECK(!res.headers().get(http::field::content_range));
        CHECK_EQ(co_await res.as_string(), data);
        co_return;
      });
  server.serve_request("/",
                       test_request::get()
                           .set_header(http::field::if_none_match, R"(""")")
                           .build(),
                       [](test_response res) -> awaitable<void> {
                         CHECK_EQ(res.status(), http::status::bad_request);
                         CHECK_EQ(res.headers().get(http::field::accept_ranges),
                                  "bytes");
                         CHECK_EQ(res.headers().get(http::field::content_range),
                                  "bytes */1048576");
                         co_return;
                       });
  server.serve_request(
      "/",
      test_request::get()
          .set_header(http::field::if_none_match, etag_str)
          .build(),
      [&](test_response res) -> awaitable<void> {
        CHECK_EQ(res.status(), http::status::not_modified);
        CHECK_EQ(res.headers().get(http::field::last_modified), lmd_str);
        CHECK_EQ(res.headers().get(http::field::etag), etag_str);
        co_return;
      });
  server.serve_request(
      "/",
      test_request::get().set_header(http::field::if_none_match, "*").build(),
      [&](test_response res) -> awaitable<void> {
        CHECK_EQ(res.status(), http::status::not_modified);
        CHECK_EQ(res.headers().get(http::field::last_modified), lmd_str);
        CHECK_EQ(res.headers().get(http::field::etag), etag_str);
        co_return;
      });
  server.serve_request(
      "/",
      test_request::get()
          .set_header(
              http::field::if_none_match,
              http::header::entity_tag::make_strong("xxxxxx").to_string())
          .build(),
      [&](test_response res) -> awaitable<void> {
        CHECK_EQ(res.status(), http::status::ok);
        CHECK_EQ(res.headers().get(http::field::last_modified), lmd_str);
        CHECK_EQ(res.headers().get(http::field::etag), etag_str);
        CHECK_EQ(res.headers().get(http::field::content_type),
                 mime::application_octet_stream());
        CHECK_EQ(res.headers().get(http::field::content_disposition), cd_str);
        CHECK_EQ(res.headers().get(http::field::accept_ranges), "bytes");
        CHECK(!res.headers().get(http::field::content_range));
        CHECK_EQ(co_await res.as_string(), data);
        co_return;
      });

  ioc.run();
}

TEST_CASE("open with request header: if_modified_since")
{
  const auto file_path = get_temp_file_path();
  const auto data = get_random_string(1048576);
  {
    std::ofstream(file_path, std::ios::binary) << data;
  }
  const auto lmd = std::filesystem::last_write_time(file_path);
  const auto lmd_str = http::header::date(lmd).to_string();
  const auto cd_str
      = fmt::format(R"(attachment; filename="{}")",
                    std::filesystem::path(file_path).filename().string());
  const auto etag_str = get_etag(data.size(), lmd).to_string();

  auto ioc = net::io_context();
  auto server
      = http_server::builder(ioc)
            .serve(route::get<"/">(
                [&file_path](const request& req)
                    -> awaitable<std::variant<static_file, response>> {
                  if (auto file = static_file::open(
                          co_await net::this_coro::executor, file_path, req);
                      file) {
                    co_return std::move(*file);
                  }

                  co_return response::not_found().build();
                }))
            .build();

  server.serve_request("/",
                       test_request::get()
                           .set_header(http::field::if_modified_since, R"(""")")
                           .build(),
                       [](test_response res) -> awaitable<void> {
                         CHECK_EQ(res.status(), http::status::bad_request);
                         CHECK_EQ(res.headers().get(http::field::accept_ranges),
                                  "bytes");
                         CHECK_EQ(res.headers().get(http::field::content_range),
                                  "bytes */1048576");
                         co_return;
                       });
  server.serve_request(
      "/",
      test_request::get()
          .set_header(
              http::field::if_modified_since,
              http::header::date(lmd + std::chrono::seconds(1)).to_string())
          .build(),
      [&](test_response res) -> awaitable<void> {
        CHECK_EQ(res.status(), http::status::not_modified);
        CHECK_EQ(res.headers().get(http::field::last_modified), lmd_str);
        CHECK_EQ(res.headers().get(http::field::etag), etag_str);
        co_return;
      });
  server.serve_request(
      "/",
      test_request::get()
          .set_header(
              http::field::if_modified_since,
              http::header::date(lmd - std::chrono::seconds(1)).to_string())
          .build(),
      [&](test_response res) -> awaitable<void> {
        CHECK_EQ(res.status(), http::status::ok);
        CHECK_EQ(res.headers().get(http::field::last_modified), lmd_str);
        CHECK_EQ(res.headers().get(http::field::etag), etag_str);
        CHECK_EQ(res.headers().get(http::field::content_type),
                 mime::application_octet_stream());
        CHECK_EQ(res.headers().get(http::field::content_disposition), cd_str);
        CHECK_EQ(res.headers().get(http::field::accept_ranges), "bytes");
        CHECK(!res.headers().get(http::field::content_range));
        CHECK_EQ(co_await res.as_string(), data);
        co_return;
      });

  server.serve_request(
      "/",
      test_request::get()
          .set_header(http::field::if_none_match, etag_str)
          .set_header(
              http::field::if_modified_since,
              http::header::date(lmd - std::chrono::seconds(1)).to_string())
          .build(),
      [&](test_response res) -> awaitable<void> {
        CHECK_EQ(res.status(), http::status::not_modified);
        CHECK_EQ(res.headers().get(http::field::last_modified), lmd_str);
        CHECK_EQ(res.headers().get(http::field::etag), etag_str);
        co_return;
      });

  ioc.run();
}

#endif

TEST_SUITE_END();
