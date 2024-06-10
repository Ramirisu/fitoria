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

#include <fitoria/http.hpp>
#include <fitoria/mime.hpp>

#include <fitoria/web/detail/as_json.hpp>

#include <fitoria/web/any_body.hpp>
#include <fitoria/web/async_message_parser_stream.hpp>
#include <fitoria/web/async_read_into_stream_file.hpp>
#include <fitoria/web/async_read_until_eof.hpp>
#include <fitoria/web/async_readable_stream_concept.hpp>
#include <fitoria/web/async_readable_vector_stream.hpp>
#include <fitoria/web/async_write_chunks.hpp>
#include <fitoria/web/error.hpp>
#include <fitoria/web/query_map.hpp>

FITORIA_NAMESPACE_BEGIN

namespace test {

using web::query_map;

class http_client {
  using duration_type = std::chrono::steady_clock::duration;

  struct resource {
    std::string host;
    std::uint16_t port;
    std::string path;
  };

public:
  class http_response {
  public:
    http_response(http::status_code status_code,
                  http::version version,
                  http::header header,
                  web::any_async_readable_stream body)
        : status_code_(status_code)
        , version_(version)
        , header_(std::move(header))
        , body_(std::move(body))
    {
    }

    http_response(const http_response&) = delete;

    http_response& operator=(const http_response&) = delete;

    http_response(http_response&&) = default;

    http_response& operator=(http_response&&) = default;

    auto status_code() const noexcept -> const http::status_code&
    {
      return status_code_;
    }

    auto version() const noexcept -> const http::version&
    {
      return version_;
    }

    auto header() const noexcept -> const http::header&
    {
      return header_;
    }

    auto body() noexcept -> web::any_async_readable_stream&
    {
      return body_;
    }

    auto body() const noexcept -> const web::any_async_readable_stream&
    {
      return body_;
    }

    auto as_string() -> awaitable<expected<std::string, std::error_code>>
    {
      return web::async_read_until_eof<std::string>(body_);
    }

    template <typename Byte>
    auto as_vector() -> awaitable<expected<std::vector<Byte>, std::error_code>>
    {
      return web::async_read_until_eof<std::vector<Byte>>(body_);
    }

#if defined(BOOST_ASIO_HAS_FILE)
    auto as_file(const std::string& path)
        -> awaitable<expected<std::size_t, std::error_code>>
    {
      auto file = net::stream_file(co_await net::this_coro::executor);

      boost::system::error_code ec; // NOLINTNEXTLINE
      file.open(path, net::file_base::create | net::file_base::write_only, ec);
      if (ec) {
        co_return unexpected { ec };
      }

      co_return co_await async_read_into_stream_file(body_, file);
    }
#endif

    template <typename T = boost::json::value>
    auto as_json() -> awaitable<expected<T, std::error_code>>
    {
      if (auto mime = header()
                          .get(http::field::content_type)
                          .and_then(mime::mime_view::parse);
          !mime || mime->essence() != mime::application_json()) {
        co_return unexpected { make_error_code(
            web::error::unexpected_content_type) };
      }

      if (auto str = co_await web::async_read_until_eof<std::string>(body_);
          str) {
        co_return web::detail::as_json<T>(*str);
      } else {
        co_return unexpected { str.error() };
      }
    }

  private:
    http::status_code status_code_ = http::status::ok;
    http::version version_ = http::version::v1_1;
    http::header header_;
    web::any_async_readable_stream body_;
  };

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

  auto header() noexcept -> http::header&
  {
    return header_;
  }

  auto header() const noexcept -> const http::header&
  {
    return header_;
  }

  auto set_header(http::field name, std::string_view value) & -> http_client&
  {
    header_.set(name, value);
    return *this;
  }

  auto set_header(http::field name, std::string_view value) && -> http_client&&
  {
    set_header(name, value);
    return std::move(*this);
  }

  auto set_header(std::string_view name,
                  std::string_view value) & -> http_client&
  {
    header_.set(name, value);
    return *this;
  }

  auto set_header(std::string_view name,
                  std::string_view value) && -> http_client&&
  {
    set_header(name, value);
    return std::move(*this);
  }

  auto insert_header(http::field name, std::string_view value) & -> http_client&
  {
    header_.insert(name, value);
    return *this;
  }

  auto insert_header(http::field name,
                     std::string_view value) && -> http_client&&
  {
    insert_header(name, value);
    return std::move(*this);
  }

  auto insert_header(std::string name, std::string_view value) & -> http_client&
  {
    header_.insert(name, value);
    return *this;
  }

  auto insert_header(std::string name,
                     std::string_view value) && -> http_client&&
  {
    insert_header(name, value);
    return std::move(*this);
  }

  template <std::size_t N>
  auto set_body(std::span<const std::byte, N> bytes) & -> http_client&
  {
    body_ = web::any_body(web::any_body::sized { bytes.size() },
                          web::async_readable_vector_stream(bytes));
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

  template <web::async_readable_stream AsyncReadableStream>
  auto set_body(AsyncReadableStream&& stream) & -> http_client&
  {
    body_ = web::any_body(web::any_body::sized { nullopt },
                          std::forward<AsyncReadableStream>(stream));
    return *this;
  }

  template <web::async_readable_stream AsyncReadableStream>
  auto set_body(AsyncReadableStream&& stream) && -> http_client&&
  {
    setbody(std::forward<AsyncReadableStream>(stream));
    return std::move(*this);
  }

  auto set_plaintext(std::string_view sv) & -> http_client&
  {
    set_header(http::field::content_type, mime::text_plain());
    return set_body(std::as_bytes(std::span(sv.begin(), sv.end())));
  }

  auto set_plaintext(std::string_view sv) && -> http_client&&
  {
    set_plaintext(sv);
    return std::move(*this);
  }

  auto set_json(const boost::json::value& jv) & -> http_client&
  {
    auto str = boost::json::serialize(jv);
    set_header(http::field::content_type, mime::application_json());
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

  template <web::async_readable_stream AsyncReadableStream>
  auto set_stream_body(AsyncReadableStream&& stream) & -> http_client&
  {
    body_ = web::any_body(web::any_body::chunked(),
                          std::forward<AsyncReadableStream>(stream));
    return *this;
  }

  template <web::async_readable_stream AsyncReadableStream>
  auto set_stream_body(AsyncReadableStream&& stream) && -> http_client&&
  {
    set_stream_body(std::forward<AsyncReadableStream>(stream));
    return std::move(*this);
  }

  auto handshake_timeout() const noexcept -> optional<duration_type>
  {
    return handshake_timeout_;
  }

  auto set_handshake_timeout(optional<duration_type> timeout) & noexcept
      -> http_client&
  {
    handshake_timeout_ = timeout;
    return *this;
  }

  auto set_handshake_timeout(optional<duration_type> timeout) && noexcept
      -> http_client&&
  {
    set_handshake_timeout(timeout);
    return std::move(*this);
  }

  auto transfer_timeout() const noexcept -> optional<duration_type>
  {
    return transfer_timeout_;
  }

  auto set_transfer_timeout(optional<duration_type> timeout) & noexcept
      -> http_client&
  {
    transfer_timeout_ = timeout;
    return *this;
  }

  auto set_transfer_timeout(optional<duration_type> timeout) && noexcept
      -> http_client&&
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

    auto stream = basic_stream<net::ip::tcp>(co_await net::this_coro::executor);

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

    auto stream
        = ssl_stream<net::ip::tcp>(co_await net::this_coro::executor, ssl_ctx);

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

  template <typename Protocol>
  auto do_handshake(ssl_stream<Protocol>& stream)
      -> awaitable<expected<void, std::error_code>>
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
                [&](web::any_body::null) { return do_sized_request(stream); },
                [&](web::any_body::sized) { return do_sized_request(stream); },
                [&](web::any_body::chunked) {
                  return do_chunked_request(stream);
                } },
            body_.size());
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

    co_return http_response(
        parser->get().result(),
        http::detail::from_impl_version(parser->get().version()),
        http::header::from_impl(parser->get()),
        [&]() -> web::any_async_readable_stream {
          if (parser->get().has_content_length() || parser->get().chunked()) {
            return web::async_message_parser_stream(
                buffer, std::move(stream), parser);
          }
          return web::async_readable_vector_stream();
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
    header_.to_impl(req);
    req.set(http::field::host, resource_->host);
    if (auto data = co_await web::async_read_until_eof<bytes>(body_.stream());
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

    if (auto field = header_.get(http::field::expect);
        field && iequals(*field, "100-continue")) {
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
    header_.to_impl(req);
    req.set(http::field::host, resource_->host);
    req.chunked(true);

    auto serializer = request_serializer<empty_body>(req);
    if (auto bytes_written
        = co_await async_write_header(stream, serializer, use_awaitable);
        !bytes_written) {
      co_return unexpected { bytes_written.error() };
    }

    if (auto field = header_.get(http::field::expect);
        field && iequals(*field, "100-continue")) {
      if (auto res = co_await handle_expect_100_continue(stream);
          !res || *res) {
        co_return res;
      }
    }

    if (auto res = co_await web::async_write_chunks(stream, body_.stream());
        !res) {
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
          http::detail::from_impl_version(res.version()),
          http::header::from_impl(res),
          web::async_readable_vector_stream(std::move(res.body())));
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
  http::header header_;
  web::any_body body_;
  optional<duration_type> handshake_timeout_ = std::chrono::seconds(3);
  optional<duration_type> transfer_timeout_ = std::chrono::seconds(5);
};
}

FITORIA_NAMESPACE_END

#endif
