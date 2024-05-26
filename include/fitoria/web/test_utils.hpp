//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_WEB_TEST_UTILS_HPP
#define FITORIA_WEB_TEST_UTILS_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/web/test_request.hpp>
#include <fitoria/web/test_response.hpp>

FITORIA_NAMESPACE_BEGIN

namespace web {

inline std::string encoded_target(std::string_view path,
                                  std::string_view query_string)
{
  boost::urls::url url;
  url.set_path(path);
  url.set_query(query_string);
  return std::string(url.encoded_target());
}

template <typename Stream>
auto handle_expect_100_continue(Stream& stream)
    -> awaitable<expected<optional<test_response>, std::error_code>>
{
  using boost::beast::error;
  using boost::beast::http::response;
  using boost::beast::http::vector_body;

  flat_buffer buffer;
  response<vector_body<std::byte>> res;
  auto bytes_read = co_await async_read(stream, buffer, res, use_awaitable);
  if (!bytes_read && bytes_read.error() != error::timeout) {
    co_return unexpected { bytes_read.error() };
  }
  if (bytes_read && res.result() != http::status::continue_) {
    co_return test_response(
        res.result(),
        http::detail::from_impl_version(res.version()),
        http::header::from_impl(res),
        async_readable_vector_stream(std::move(res.body())));
  }

  co_return nullopt;
}

template <typename Stream>
auto do_sized_request(Stream& stream, std::string path, test_request& tr)
    -> awaitable<expected<optional<test_response>, std::error_code>>
{
  using boost::beast::http::request;
  using boost::beast::http::request_serializer;
  using boost::beast::http::vector_body;

  auto req = request<vector_body<std::byte>>(
      tr.method(), encoded_target(path, tr.query().to_string()), 11);
  tr.header().to_impl(req);
  if (auto data = co_await async_read_until_eof<std::vector<std::byte>>(
          tr.body().stream());
      data) {
    req.body() = std::move(*data);
  } else if (data.error() != make_error_code(net::error::eof)) {
    co_return unexpected { data.error() };
  }
  req.prepare_payload();

  auto ser = request_serializer<vector_body<std::byte>>(req);
  if (auto bytes_written
      = co_await async_write_header(stream, ser, use_awaitable);
      !bytes_written) {
    co_return unexpected { bytes_written.error() };
  }

  if (auto field = tr.header().get(http::field::expect);
      field && iequals(*field, "100-continue")) {
    if (auto res = co_await handle_expect_100_continue(stream); !res || *res) {
      co_return res;
    }
  }

  if (auto bytes_written = co_await async_write(stream, ser, use_awaitable);
      !bytes_written) {
    co_return unexpected { bytes_written.error() };
  }

  co_return nullopt;
}

template <typename Stream>
auto do_chunked_request(Stream& stream, std::string path, test_request& tr)
    -> awaitable<expected<optional<test_response>, std::error_code>>
{
  using boost::beast::http::empty_body;
  using boost::beast::http::request;
  using boost::beast::http::request_serializer;

  auto req = request<empty_body>(
      tr.method(), encoded_target(path, tr.query().to_string()), 11);
  tr.header().to_impl(req);
  req.chunked(true);

  auto serializer = request_serializer<empty_body>(req);
  if (auto bytes_written
      = co_await async_write_header(stream, serializer, use_awaitable);
      !bytes_written) {
    co_return unexpected { bytes_written.error() };
  }

  if (auto field = tr.header().get(http::field::expect);
      field && iequals(*field, "100-continue")) {
    if (auto res = co_await handle_expect_100_continue(stream); !res || *res) {
      co_return res;
    }
  }

  if (auto res = co_await async_write_chunks(stream, tr.body().stream());
      !res) {
    co_return unexpected { res.error() };
  }

  co_return nullopt;
}

template <typename Stream>
auto do_http_request(Stream stream, std::string path, test_request& req)
    -> awaitable<expected<test_response, std::error_code>>
{
  using boost::beast::http::buffer_body;
  using boost::beast::http::response_parser;

  if (auto exp = co_await std::visit(
          overloaded { [&](any_body::null) {
                        return do_sized_request(stream, path, req);
                      },
                       [&](any_body::sized) {
                         return do_sized_request(stream, path, req);
                       },
                       [&](any_body::chunked) {
                         return do_chunked_request(stream, path, req);
                       } },
          req.body().size());
      !exp) {
    co_return unexpected { exp.error() };
  } else if (*exp) {
    co_return std::move(**exp);
  }

  auto buffer = std::make_shared<flat_buffer>();
  auto parser = std::make_shared<response_parser<buffer_body>>();
  if (auto bytes_read
      = co_await async_read_header(stream, *buffer, *parser, use_awaitable);
      !bytes_read) {
    co_return unexpected { bytes_read.error() };
  }

  co_return test_response(
      parser->get().result(),
      http::detail::from_impl_version(parser->get().version()),
      http::header::from_impl(parser->get()),
      [&]() -> any_async_readable_stream {
        if (parser->get().has_content_length() || parser->get().chunked()) {
          return async_message_parser_stream(buffer, std::move(stream), parser);
        }
        return async_readable_vector_stream();
      }());
}

}

FITORIA_NAMESPACE_END

#endif
