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

#include <fitoria/core/error.hpp>
#include <fitoria/core/expected.hpp>
#include <fitoria/core/json.hpp>
#include <fitoria/core/net.hpp>
#include <fitoria/core/url.hpp>

#include <fitoria/log/log.hpp>

#include <fitoria/web/async_message_parser_stream.hpp>
#include <fitoria/web/async_readable_stream_concept.hpp>
#include <fitoria/web/async_readable_vector_stream.hpp>
#include <fitoria/web/http/http.hpp>
#include <fitoria/web/http_fields.hpp>
#include <fitoria/web/http_response.hpp>
#include <fitoria/web/query_map.hpp>

FITORIA_NAMESPACE_BEGIN

namespace client {

namespace http = web::http;

using web::any_async_readable_stream;
using web::async_message_parser_stream;
using web::async_readable_stream;
using web::async_readable_vector_stream;
using web::http_fields;
using web::http_response;
using web::query_map;

class http_client {
  struct resource {
    std::string host;
    std::uint16_t port;
    std::string path;
  };

public:
  http_client(http::verb method,
              std::string host,
              std::uint16_t port,
              std::string path)
      : resource_(resource { std::move(host), port, std::move(path) })
      , method_(method)
  {
  }

  http_client(http::verb method, std::string_view url)
      : resource_(parse_uri(url))
      , method_(method)
  {
  }

  static http_client get(std::string_view url)
  {
    return http_client(http::verb::get, url);
  }

  static http_client post(std::string_view url)
  {
    return http_client(http::verb::post, url);
  }

  static http_client put(std::string_view url)
  {
    return http_client(http::verb::put, url);
  }

  static http_client patch(std::string_view url)
  {
    return http_client(http::verb::patch, url);
  }

  static http_client delete_(std::string_view url)
  {
    return http_client(http::verb::delete_, url);
  }

  static http_client head(std::string_view url)
  {
    return http_client(http::verb::head, url);
  }

  static http_client options(std::string_view url)
  {
    return http_client(http::verb::options, url);
  }

  expected<const std::string&, error_code> host() const noexcept
  {
    return resource_.transform(
        [](const resource& res) -> const std::string& { return res.host; });
  }

  expected<std::uint16_t, error_code> port() const noexcept
  {
    return resource_.transform([](const resource& res) { return res.port; });
  }

  expected<const std::string&, error_code> path() const noexcept
  {
    return resource_.transform(
        [](const resource& res) -> const std::string& { return res.path; });
  }

  query_map& query() noexcept
  {
    return query_;
  }

  const query_map& query() const noexcept
  {
    return query_;
  }

  http_client& set_query(std::string name, std::string value)
  {
    query_.set(std::move(name), std::move(value));
    return *this;
  }

  http::verb method() const noexcept
  {
    return method_;
  }

  http_client& set_method(http::verb method)
  {
    method_ = method;
    return *this;
  }

  http_fields& fields() noexcept
  {
    return fields_;
  }

  const http_fields& fields() const noexcept
  {
    return fields_;
  }

  http_client& set_field(http::field name, std::string_view value)
  {
    fields_.set(name, value);
    return *this;
  }

  http_client& set_field(std::string name, std::string_view value)
  {
    fields_.set(std::move(name), value);
    return *this;
  }

  http_client& insert_field(http::field name, std::string_view value)
  {
    fields_.insert(name, value);
    return *this;
  }

  http_client& insert_field(std::string name, std::string_view value)
  {
    fields_.insert(std::move(name), value);
    return *this;
  }

  template <std::size_t N>
  http_client& set_body(std::span<const std::byte, N> bytes)
  {
    body_ = async_readable_vector_stream(bytes);
    return *this;
  }

  http_client& set_body(std::string_view sv)
  {
    set_body(std::as_bytes(std::span(sv.begin(), sv.end())));
    return *this;
  }

  http_client& set_plaintext(std::string_view sv)
  {
    set_field(http::field::content_type,
              http::fields::content_type::plaintext());
    return set_body(std::as_bytes(std::span(sv.begin(), sv.end())));
  }

  template <typename T = boost::json::value>
  http_client& set_json(const T& obj)
  {
    if constexpr (std::is_same_v<T, boost::json::value>) {
      set_field(http::field::content_type, http::fields::content_type::json());
      auto s = boost::json::serialize(obj);
      set_body(std::as_bytes(std::span(s.begin(), s.end())));
    } else {
      set_json(boost::json::value_from(obj));
    }
    return *this;
  }

  template <async_readable_stream AsyncReadableStream>
  http_client& set_stream(AsyncReadableStream&& stream)
  {
    body_ = std::forward<AsyncReadableStream>(stream);
    return *this;
  }

  std::chrono::milliseconds request_timeout() const noexcept
  {
    return request_timeout_;
  }

  http_client& set_request_timeout(std::chrono::milliseconds timeout) noexcept
  {
    request_timeout_ = timeout;
    return *this;
  }

  std::chrono::milliseconds expect_100_timeout() const noexcept
  {
    return expect100_timeout_;
  }

  http_client&
  set_expect_100_timeout(std::chrono::milliseconds timeout) noexcept
  {
    expect100_timeout_ = timeout;
    return *this;
  }

  auto async_send() -> net::awaitable<expected<http_response, error_code>>
  {
    if (!resource_) {
      co_return unexpected { resource_.error() };
    }

    co_return co_await do_session();
  }

#if defined(FITORIA_HAS_OPENSSL)
  auto async_send(net::ssl::context ssl_ctx)
      -> net::awaitable<expected<http_response, error_code>>
  {
    if (!resource_) {
      co_return unexpected { resource_.error() };
    }

    co_return co_await do_session(std::move(ssl_ctx));
  }
#endif

  const char* name() const noexcept
  {
    return "fitoria.client.http_client";
  }

private:
  static expected<resource, error_code> parse_uri(std::string_view url)
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

  auto do_resolve() const -> net::awaitable<
      expected<net::ip::basic_resolver_results<net::ip::tcp>, error_code>>
  {
    auto resolver = net::ip::tcp::resolver(co_await net::this_coro::executor);
    auto [ec, results] = co_await resolver.async_resolve(
        resource_->host, std::to_string(resource_->port), net::use_ta);
    if (ec) {
      log::debug("[{}] async_resolve failed: {}", name(), ec.message());
      co_return unexpected { ec };
    }

    co_return results;
  }

  auto do_session() -> net::awaitable<expected<http_response, error_code>>
  {
    auto results = co_await do_resolve();
    if (!results) {
      co_return unexpected { results.error() };
    }

    auto stream = net::shared_tcp_stream(co_await net::this_coro::executor);

    stream->expires_after(request_timeout_);
    auto [ec, _] = co_await stream->async_connect(*results, net::use_ta);
    if (ec) {
      log::debug("[{}] async_connect failed: {}", name(), ec.message());
      co_return unexpected { ec };
    }

    co_return co_await do_send_recv(stream);
  }

#if defined(FITORIA_HAS_OPENSSL)
  auto do_session(net::ssl::context ssl_ctx)
      -> net::awaitable<expected<http_response, error_code>>
  {
    auto results = co_await do_resolve();
    if (!results) {
      co_return unexpected { results.error() };
    }

    boost::system::error_code ec;
    auto stream = net::shared_ssl_stream(
        co_await net::this_coro::executor,
        std::make_shared<net::ssl::context>(std::move(ssl_ctx)));

    // Set SNI Hostname (many hosts need this to handshake successfully)
    if (!SSL_set_tlsext_host_name(stream->native_handle(),
                                  resource_->host.c_str())) {
      co_return unexpected { boost::system::error_code(
          static_cast<int>(::ERR_get_error()),
          net::error::get_ssl_category()) };
    }

    boost::beast::get_lowest_layer(*stream).expires_after(request_timeout_);
    std::tie(ec, std::ignore)
        = co_await boost::beast::get_lowest_layer(*stream).async_connect(
            *results, net::use_ta);
    if (ec) {
      log::debug("[{}] async_connect failed: {}", name(), ec.message());
      co_return unexpected { ec };
    }

    boost::beast::get_lowest_layer(*stream).expires_after(request_timeout_);
    std::tie(ec) = co_await stream->async_handshake(
        net::ssl::stream_base::client, net::use_ta);
    if (ec) {
      log::debug("[{}] async_handshake failed: {}", name(), ec.message());
      co_return unexpected { ec };
    }

    co_return co_await do_send_recv(stream);
  }
#endif

  template <typename Stream>
  auto do_send_recv(Stream& stream)
      -> net::awaitable<expected<http_response, error_code>>
  {
    using boost::beast::http::buffer_body;
    using boost::beast::http::response_parser;

    auto do_request =
        [&]() -> net::awaitable<expected<optional<http_response>, error_code>> {
      return body_.size_hint() ? do_sized_request(stream)
                               : do_chunked_request(stream);
    };
    if (auto exp = co_await do_request(); !exp) {
      co_return unexpected { exp.error() };
    } else if (*exp) {
      co_return std::move(**exp);
    }

    boost::system::error_code ec;
    boost::beast::flat_buffer buffer;

    auto parser = std::make_unique<response_parser<buffer_body>>();
    boost::beast::get_lowest_layer(*stream).expires_after(request_timeout_);
    std::tie(ec, std::ignore)
        = co_await async_read_header(*stream, buffer, *parser, net::use_ta);
    if (ec) {
      log::debug("[{}] async_read_header failed: {}", name(), ec.message());
      co_return unexpected { ec };
    }

    auto res = http_response(parser->get().result(),
                             http_fields::from(parser->get()));
    res.set_stream(async_message_parser_stream(std::move(buffer),
                                               std::move(stream),
                                               std::move(parser),
                                               request_timeout_));
    co_return res;
  }

  template <typename Stream>
  auto do_sized_request(Stream& stream)
      -> net::awaitable<expected<optional<http_response>, error_code>>
  {
    using boost::beast::http::request;
    using boost::beast::http::request_serializer;
    using boost::beast::http::response;
    using boost::beast::http::vector_body;

    bool use_expect = fields_.get(http::field::expect) == "100-continue";

    request<vector_body<std::byte>> req(
        method_, encoded_target(resource_->path, query_.to_string()), 11);
    fields_.to(req);
    req.set(http::field::host, resource_->host);
    if (auto data
        = co_await web::async_read_all_as<std::vector<std::byte>>(body_);
        data) {
      if (!*data) {
        co_return unexpected { (*data).error() };
      }
      req.body() = std::move(**data);
    }
    req.prepare_payload();

    boost::system::error_code ec;

    auto serializer = request_serializer<vector_body<std::byte>>(req);
    boost::beast::get_lowest_layer(*stream).expires_after(request_timeout_);
    std::tie(ec, std::ignore)
        = co_await async_write_header(*stream, serializer, net::use_ta);
    if (ec) {
      log::debug("[{}] async_write_header failed: {}", name(), ec.message());
      co_return unexpected { ec };
    }

    if (use_expect) {
      boost::beast::flat_buffer buffer;
      response<vector_body<std::byte>> res;
      boost::beast::get_lowest_layer(*stream).expires_after(expect100_timeout_);
      std::tie(ec, std::ignore)
          = co_await async_read(*stream, buffer, res, net::use_ta);
      if (ec && ec != boost::beast::error::timeout) {
        log::debug("[{}] async_read failed: {}", name(), ec.message());
        co_return unexpected { ec };
      }
      if (!ec && res.result() != http::status::continue_) {
        co_return http_response(res.result(), http_fields::from(res))
            .set_stream(async_readable_vector_stream(std::move(res.body())));
      }
    }

    boost::beast::get_lowest_layer(*stream).expires_after(request_timeout_);
    std::tie(ec, std::ignore)
        = co_await async_write(*stream, serializer, net::use_ta);
    if (ec) {
      log::debug("[{}] async_write failed: {}", name(), ec.message());
      co_return unexpected { ec };
    }

    co_return nullopt;
  }

  template <typename Stream>
  auto do_chunked_request(Stream& stream)
      -> net::awaitable<expected<optional<http_response>, error_code>>
  {
    using boost::beast::http::empty_body;
    using boost::beast::http::request;
    using boost::beast::http::request_serializer;
    using boost::beast::http::response;
    using boost::beast::http::vector_body;

    bool use_expect = fields_.get(http::field::expect) == "100-continue";

    request<empty_body> req(
        method_, encoded_target(resource_->path, query_.to_string()), 11);
    fields_.to(req);
    req.set(http::field::host, resource_->host);
    req.chunked(true);

    boost::system::error_code ec;

    auto serializer = request_serializer<empty_body>(req);
    boost::beast::get_lowest_layer(*stream).expires_after(request_timeout_);
    std::tie(ec, std::ignore)
        = co_await async_write_header(*stream, serializer, net::use_ta);
    if (ec) {
      log::debug("[{}] async_write_header failed: {}", name(), ec.message());
      co_return unexpected { ec };
    }

    if (use_expect) {
      boost::beast::flat_buffer buffer;
      response<vector_body<std::byte>> res;
      boost::beast::get_lowest_layer(*stream).expires_after(expect100_timeout_);
      std::tie(ec, std::ignore)
          = co_await async_read(*stream, buffer, res, net::use_ta);
      if (ec && ec != boost::beast::error::timeout) {
        log::debug("[{}] async_read failed: {}", name(), ec.message());
        co_return unexpected { ec };
      }
      if (!ec && res.result() != http::status::continue_) {
        co_return http_response(res.result(), http_fields::from(res))
            .set_stream(async_readable_vector_stream(std::move(res.body())));
      }
    }

    if (auto res = co_await web::async_write_each_chunk(
            *stream, body_, request_timeout_);
        !res) {
      log::debug(
          "[{}] async_write_each_chunk failed: {}", name(), ec.message());
      co_return unexpected { res.error() };
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

  expected<resource, error_code> resource_;
  query_map query_;
  http::verb method_ = http::verb::unknown;
  http_fields fields_;
  any_async_readable_stream body_ { async_readable_vector_stream::eof() };
  std::chrono::milliseconds request_timeout_ = std::chrono::seconds(5);
  std::chrono::milliseconds expect100_timeout_ = std::chrono::seconds(1);
};
}

FITORIA_NAMESPACE_END

#endif
