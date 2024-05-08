//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_CLIENT_HTTP_CLIENT_HPP
#define FITORIA_CLIENT_HTTP_CLIENT_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/core/expected.hpp>
#include <fitoria/core/json.hpp>
#include <fitoria/core/net.hpp>
#include <fitoria/core/url.hpp>
#include <fitoria/core/utility.hpp>

#include <fitoria/log/log.hpp>

#include <fitoria/web/any_body.hpp>
#include <fitoria/web/async_message_parser_stream.hpp>
#include <fitoria/web/async_readable_stream_concept.hpp>
#include <fitoria/web/async_readable_vector_stream.hpp>
#include <fitoria/web/async_write_chunks.hpp>
#include <fitoria/web/http/http.hpp>
#include <fitoria/web/http_fields.hpp>
#include <fitoria/web/query_map.hpp>

#include <fitoria/client/http_response.hpp>

FITORIA_NAMESPACE_BEGIN

namespace client {

namespace http = web::http;

using web::any_async_readable_stream;
using web::any_body;
using web::async_message_parser_stream;
using web::async_read_until_eof;
using web::async_readable_stream;
using web::async_readable_vector_stream;
using web::async_write_chunks;
using web::http_fields;
using web::query_map;

class http_client {
  struct resource {
    std::string host;
    std::uint16_t port;
    std::string path;
  };

public:
  auto set_url(std::string_view url) & -> http_client&
  {
    resource_ = parse_uri(url);
    return *this;
  }

  auto set_url(std::string_view url) && -> http_client&&
  {
    set_url(url);
    return std::move(*this);
  }

  auto set_url(std::string host,
               std::uint16_t port,
               std::string path) & -> http_client&
  {
    resource_ = resource { .host = std::move(host),
                           .port = port,
                           .path = std::move(path) };
    return *this;
  }

  auto set_url(std::string host,
               std::uint16_t port,
               std::string path) && -> http_client&&
  {
    set_url(std::move(host), port, std::move(path));
    return std::move(*this);
  }

  auto host() const noexcept -> expected<const std::string&, std::error_code>
  {
    return resource_.transform(
        [](const resource& res) -> const std::string& { return res.host; });
  }

  auto port() const noexcept -> expected<std::uint16_t, std::error_code>
  {
    return resource_.transform([](const resource& res) { return res.port; });
  }

  auto path() const noexcept -> expected<const std::string&, std::error_code>
  {
    return resource_.transform(
        [](const resource& res) -> const std::string& { return res.path; });
  }

  auto query() noexcept -> query_map&
  {
    return query_;
  }

  auto query() const noexcept -> const query_map&
  {
    return query_;
  }

  auto set_query(std::string name, std::string value) & -> http_client&
  {
    query_.set(std::move(name), std::move(value));
    return *this;
  }

  auto set_query(std::string name, std::string value) && -> http_client&&
  {
    set_query(std::move(name), std::move(value));
    return std::move(*this);
  }

  auto method() const noexcept -> http::verb
  {
    return method_;
  }

  auto set_method(http::verb method) & -> http_client&
  {
    method_ = method;
    return *this;
  }

  auto set_method(http::verb method) && -> http_client&&
  {
    set_method(method);
    return std::move(*this);
  }

  auto fields() noexcept -> http_fields&
  {
    return fields_;
  }

  auto fields() const noexcept -> const http_fields&
  {
    return fields_;
  }

  auto set_field(http::field name, std::string_view value) & -> http_client&
  {
    fields_.set(name, value);
    return *this;
  }

  auto set_field(http::field name, std::string_view value) && -> http_client&&
  {
    set_field(name, value);
    return std::move(*this);
  }

  auto set_field(std::string_view name,
                 std::string_view value) & -> http_client&
  {
    fields_.set(name, value);
    return *this;
  }

  auto set_field(std::string_view name,
                 std::string_view value) && -> http_client&&
  {
    set_field(name, value);
    return std::move(*this);
  }

  auto insert_field(http::field name, std::string_view value) & -> http_client&
  {
    fields_.insert(name, value);
    return *this;
  }

  auto insert_field(http::field name,
                    std::string_view value) && -> http_client&&
  {
    insert_field(name, value);
    return std::move(*this);
  }

  auto insert_field(std::string name, std::string_view value) & -> http_client&
  {
    fields_.insert(name, value);
    return *this;
  }

  auto insert_field(std::string name,
                    std::string_view value) && -> http_client&&
  {
    insert_field(name, value);
    return std::move(*this);
  }

  template <std::size_t N>
  auto set_body(std::span<const std::byte, N> bytes) & -> http_client&
  {
    body_ = any_body(any_body::sized { bytes.size() },
                     async_readable_vector_stream(bytes));
    return *this;
  }

  template <std::size_t N>
  auto set_body(std::span<const std::byte, N> bytes) && -> http_client&&
  {
    set_body(bytes);
    return std::move(*this);
  }

  auto set_body(std::string_view sv) & -> http_client&
  {
    set_body(std::as_bytes(std::span(sv.begin(), sv.end())));
    return *this;
  }

  auto set_body(std::string_view sv) && -> http_client&&
  {
    set_body(std::as_bytes(std::span(sv.begin(), sv.end())));
    return std::move(*this);
  }

  auto set_plaintext(std::string_view sv) & -> http_client&
  {
    set_field(http::field::content_type,
              http::fields::content_type::plaintext());
    return set_body(std::as_bytes(std::span(sv.begin(), sv.end())));
  }

  auto set_plaintext(std::string_view sv) && -> http_client&&
  {
    set_plaintext(sv);
    return std::move(*this);
  }

  auto set_json(const boost::json::value& jv) & -> http_client&
  {
    set_field(http::field::content_type, http::fields::content_type::json());
    auto str = boost::json::serialize(jv);
    set_body(std::as_bytes(std::span(str.begin(), str.end())));
    return *this;
  }

  auto set_json(const boost::json::value& jv) && -> http_client&&
  {
    set_json(jv);
    return std::move(*this);
  }

  template <typename T>
    requires(boost::json::has_value_from<T>::value)
  auto set_json(const T& obj) & -> http_client&
  {
    return set_json(boost::json::value_from(obj));
  }

  template <typename T>
    requires(boost::json::has_value_from<T>::value)
  auto set_json(const T& obj) && -> http_client&&
  {
    set_json(obj);
    return std::move(*this);
  }

  template <async_readable_stream AsyncReadableStream>
  auto set_stream(AsyncReadableStream&& stream) & -> http_client&
  {
    body_ = any_body(any_body::chunked(),
                     std::forward<AsyncReadableStream>(stream));
    return *this;
  }

  template <async_readable_stream AsyncReadableStream>
  auto set_stream(AsyncReadableStream&& stream) && -> http_client&&
  {
    set_stream(std::forward<AsyncReadableStream>(stream));
    return std::move(*this);
  }

  auto handshake_timeout() const noexcept -> optional<std::chrono::milliseconds>
  {
    return handshake_timeout_;
  }

  auto set_handshake_timeout(
      optional<std::chrono::milliseconds> timeout) & noexcept -> http_client&
  {
    handshake_timeout_ = timeout;
    return *this;
  }

  auto set_handshake_timeout(
      optional<std::chrono::milliseconds> timeout) && noexcept -> http_client&&
  {
    set_handshake_timeout(timeout);
    return std::move(*this);
  }

  auto transfer_timeout() const noexcept -> optional<std::chrono::milliseconds>
  {
    return transfer_timeout_;
  }

  auto set_transfer_timeout(
      optional<std::chrono::milliseconds> timeout) & noexcept -> http_client&
  {
    transfer_timeout_ = timeout;
    return *this;
  }

  auto set_transfer_timeout(
      optional<std::chrono::milliseconds> timeout) && noexcept -> http_client&&
  {
    set_transfer_timeout(timeout);
    return std::move(*this);
  }

  auto async_send() -> awaitable<expected<http_response, std::error_code>>
  {
    if (!resource_) {
      co_return unexpected { resource_.error() };
    }

    co_return co_await do_session();
  }

#if defined(FITORIA_HAS_OPENSSL)
  auto async_send(net::ssl::context& ssl_ctx)
      -> awaitable<expected<http_response, std::error_code>>
  {
    if (!resource_) {
      co_return unexpected { resource_.error() };
    }

    co_return co_await do_session(ssl_ctx);
  }
#endif

  const char* name() const noexcept
  {
    return "fitoria.client.http_client";
  }

private:
  static auto
  parse_uri(std::string_view url) -> expected<resource, std::error_code>
  {
    auto res = boost::urls::parse_uri(url);
    if (!res) {
      return unexpected { res.error() };
    }

    auto port_number = res->port_number();
    if (port_number == 0) {
      switch (res->scheme_id()) {
      case boost::urls::scheme::https:
        port_number = 443;
        break;
      case boost::urls::scheme::http:
      default:
        port_number = 80;
        break;
      }
    }

    return resource { std::string(res->host()),
                      port_number,
                      std::string(res->path()) };
  }

  auto do_resolver() const
      -> awaitable<expected<net::ip::basic_resolver_results<net::ip::tcp>,
                            std::error_code>>
  {
    auto resolver = net::ip::tcp::resolver(co_await net::this_coro::executor);

    co_return co_await resolver.async_resolve(
        resource_->host, std::to_string(resource_->port), use_awaitable);
  }

  auto do_session() -> awaitable<expected<http_response, std::error_code>>
  {
    auto results = co_await do_resolver();
    if (!results) {
      co_return unexpected { results.error() };
    }

    auto stream = tcp_stream(co_await net::this_coro::executor);

    if (auto result = co_await stream.async_connect(*results, use_awaitable);
        !result) {
      co_return unexpected { result.error() };
    }

    co_return co_await do_http_request(std::move(stream));
  }

#if defined(FITORIA_HAS_OPENSSL)
  auto do_session(net::ssl::context& ssl_ctx)
      -> awaitable<expected<http_response, std::error_code>>
  {
    auto results = co_await do_resolver();
    if (!results) {
      co_return unexpected { results.error() };
    }

    auto stream = ssl_stream(co_await net::this_coro::executor, ssl_ctx);

    // Set SNI Hostname (many hosts need this to handshake successfully)
    if (!SSL_set_tlsext_host_name(stream.native_handle(),
                                  resource_->host.c_str())) {
      co_return unexpected { boost::system::error_code(
          static_cast<int>(::ERR_get_error()),
          net::error::get_ssl_category()) };
    }

    if (auto result = co_await get_lowest_layer(stream).async_connect(
            *results, use_awaitable);
        !result) {
      co_return unexpected { result.error() };
    }

    if (auto result = co_await do_handshake(stream); !result) {
      co_return unexpected { result.error() };
    }

    co_return co_await do_http_request(std::move(stream));
  }

  auto
  do_handshake(ssl_stream& stream) -> awaitable<expected<void, std::error_code>>
  {
    using namespace net::experimental::awaitable_operators;

    auto timer = net::steady_timer(co_await net::this_coro::executor);
    if (handshake_timeout_) {
      timer.expires_after(*handshake_timeout_);
    } else {
      timer.expires_at(net::steady_timer::time_point::max());
    }

    auto result = co_await (
        stream.async_handshake(net::ssl::stream_base::client, use_awaitable)
        || timer.async_wait(use_awaitable));
    if (result.index() == 0) {
      co_return std::get<0>(result);
    }

    get_lowest_layer(stream).close();
    co_return unexpected { make_error_code(net::error::timed_out) };
  }
#endif

  template <typename Stream>
  auto do_http_request(Stream stream)
      -> awaitable<expected<http_response, std::error_code>>
  {
    using boost::beast::http::buffer_body;
    using boost::beast::http::response_parser;

    if (transfer_timeout_) {
      get_lowest_layer(stream).expires_after(*transfer_timeout_);
    }
    if (auto exp = co_await std::visit(
            overloaded {
                [&](any_body::null) { return do_sized_request(stream); },
                [&](any_body::sized) { return do_sized_request(stream); },
                [&](any_body::chunked) { return do_chunked_request(stream); } },
            body_.size());
        !exp) {
      co_return unexpected { exp.error() };
    } else if (*exp) {
      co_return std::move(**exp);
    }

    flat_buffer buffer;
    auto parser = std::make_shared<response_parser<buffer_body>>();
    if (auto bytes_read
        = co_await async_read_header(stream, buffer, *parser, use_awaitable);
        !bytes_read) {
      co_return unexpected { bytes_read.error() };
    }

    co_return http_response(
        parser->get().result(),
        http_fields::from_impl(parser->get()),
        [&]() -> any_async_readable_stream {
          if (parser->get().has_content_length() || parser->get().chunked()) {
            return async_message_parser_stream(
                std::move(buffer), std::move(stream), parser);
          }
          return async_readable_vector_stream();
        }());
  }

  template <typename Stream>
  auto do_sized_request(Stream& stream)
      -> awaitable<expected<optional<http_response>, std::error_code>>
  {
    using boost::beast::http::request;
    using boost::beast::http::request_serializer;
    using boost::beast::http::vector_body;

    auto req = request<vector_body<std::byte>>(
        method_, encoded_target(resource_->path, query_.to_string()), 11);
    fields_.to_impl(req);
    req.set(http::field::host, resource_->host);
    if (auto data
        = co_await async_read_until_eof<std::vector<std::byte>>(body_.stream());
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

    if (auto field = fields_.get(http::field::expect); field
        && iequals(*field, http::fields::expect::one_hundred_continue())) {
      if (auto res = co_await handle_expect_100_continue(stream);
          !res || *res) {
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
  auto do_chunked_request(Stream& stream)
      -> awaitable<expected<optional<http_response>, std::error_code>>
  {
    using boost::beast::http::empty_body;
    using boost::beast::http::request;
    using boost::beast::http::request_serializer;

    auto req = request<empty_body>(
        method_, encoded_target(resource_->path, query_.to_string()), 11);
    fields_.to_impl(req);
    req.set(http::field::host, resource_->host);
    req.chunked(true);

    auto serializer = request_serializer<empty_body>(req);
    if (auto bytes_written
        = co_await async_write_header(stream, serializer, use_awaitable);
        !bytes_written) {
      co_return unexpected { bytes_written.error() };
    }

    if (auto field = fields_.get(http::field::expect); field
        && iequals(*field, http::fields::expect::one_hundred_continue())) {
      if (auto res = co_await handle_expect_100_continue(stream);
          !res || *res) {
        co_return res;
      }
    }

    if (auto res = co_await async_write_chunks(stream, body_.stream()); !res) {
      co_return unexpected { res.error() };
    }

    co_return nullopt;
  }

  template <typename Stream>
  static auto handle_expect_100_continue(Stream& stream)
      -> awaitable<expected<optional<http_response>, std::error_code>>
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
      co_return http_response(
          res.result(),
          http_fields::from_impl(res),
          async_readable_vector_stream(std::move(res.body())));
    }

    co_return nullopt;
  }

  static std::string encoded_target(std::string_view path,
                                    std::string_view query_string)
  {
    boost::urls::url url;
    url.set_path(path);
    url.set_query(query_string);
    return std::string(url.encoded_target());
  }

  expected<resource, std::error_code> resource_;
  query_map query_;
  http::verb method_ = http::verb::unknown;
  http_fields fields_;
  any_body body_;
  optional<std::chrono::milliseconds> handshake_timeout_
      = std::chrono::seconds(3);
  optional<std::chrono::milliseconds> transfer_timeout_
      = std::chrono::seconds(5);
};
}

FITORIA_NAMESPACE_END

#endif
