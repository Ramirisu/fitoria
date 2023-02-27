//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef FITORIA_CLIENT_HTTP_CLIENT_HPP
#define FITORIA_CLIENT_HTTP_CLIENT_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/core/error.hpp>
#include <fitoria/core/expected.hpp>
#include <fitoria/core/json.hpp>
#include <fitoria/core/lazy.hpp>
#include <fitoria/core/net.hpp>
#include <fitoria/core/url.hpp>

#include <fitoria/log/log.hpp>

#include <fitoria/client/http_response.hpp>

#include <fitoria/web/async_stream.hpp>
#include <fitoria/web/http/http.hpp>
#include <fitoria/web/http_fields.hpp>
#include <fitoria/web/query_map.hpp>

FITORIA_NAMESPACE_BEGIN

namespace client {

namespace http = web::http;

using web::any_async_readable_stream;
using web::async_readable_stream;
using web::async_readable_vector_stream;
using web::http_fields;
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

  static http_client GET(std::string_view url)
  {
    return http_client(http::verb::get, url);
  }

  static http_client POST(std::string_view url)
  {
    return http_client(http::verb::post, url);
  }

  static http_client PUT(std::string_view url)
  {
    return http_client(http::verb::put, url);
  }

  static http_client PATCH(std::string_view url)
  {
    return http_client(http::verb::patch, url);
  }

  static http_client DELETE_(std::string_view url)
  {
    return http_client(http::verb::delete_, url);
  }

  static http_client HEAD(std::string_view url)
  {
    return http_client(http::verb::head, url);
  }

  static http_client OPTIONS(std::string_view url)
  {
    return http_client(http::verb::options, url);
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
  http_client& set_raw(std::span<const std::byte, N> bytes)
  {
    body_.emplace(async_readable_vector_stream(bytes));
    return *this;
  }

  http_client& set_plaintext(std::string_view sv)
  {
    set_field(http::field::content_type,
              http::fields::content_type::plaintext());
    return set_raw(std::as_bytes(std::span(sv.begin(), sv.end())));
  }

  template <typename T = boost::json::value>
  http_client& set_json(const T& obj)
  {
    if constexpr (std::is_same_v<T, boost::json::value>) {
      set_field(http::field::content_type, http::fields::content_type::json());
      auto s = boost::json::serialize(obj);
      set_raw(std::as_bytes(std::span(s.begin(), s.end())));
    } else {
      set_json(boost::json::value_from(obj));
    }
    return *this;
  }

  template <async_readable_stream AsyncReadableStream>
  http_client& set_stream(AsyncReadableStream&& stream)
  {
    body_.emplace(std::forward<AsyncReadableStream>(stream));
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

  auto send() const -> expected<http_response, error_code>
  {
    if (!resource_) {
      return unexpected { resource_.error() };
    }

    return net::sync_wait(do_session());
  }

#if defined(FITORIA_HAS_OPENSSL)
  auto send(net::ssl::context ssl_ctx) const
      -> expected<http_response, error_code>
  {
    if (!resource_) {
      return unexpected { resource_.error() };
    }

    return net::sync_wait(do_session(std::move(ssl_ctx)));
  }
#endif

  auto async_send() const -> lazy<expected<http_response, error_code>>
  {
    if (!resource_) {
      co_return unexpected { resource_.error() };
    }

    co_return co_await do_session();
  }

#if defined(FITORIA_HAS_OPENSSL)
  auto async_send(net::ssl::context ssl_ctx) const
      -> lazy<expected<http_response, error_code>>
  {
    if (!resource_) {
      co_return unexpected { resource_.error() };
    }

    co_return co_await do_session(std::move(ssl_ctx));
  }
#endif

  const char* name() const noexcept
  {
    return "fitoria.http_client";
  }

private:
  static std::string encode_whitespace(std::string_view s)
  {
    std::string encoded;
    for (auto& c : s) {
      if (c == ' ') {
        encoded += "%20";
      } else {
        encoded += c;
      }
    }

    return encoded;
  }

  static expected<resource, error_code> parse_uri(std::string_view url)
  {
    auto encoded_url = encode_whitespace(url); // FIXME:
    auto res = boost::urls::parse_uri(encoded_url);
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

  auto do_resolve() const -> lazy<expected<net::resolver_results, error_code>>
  {
    auto resolver = net::resolver(co_await net::this_coro::executor);
    auto [ec, results] = co_await resolver.async_resolve(
        resource_->host, std::to_string(resource_->port));
    if (ec) {
      log::debug("[{}] async_resolve failed: {}", name(), ec.message());
      co_return unexpected { ec };
    }

    co_return results;
  }

  auto do_session() const -> lazy<expected<http_response, error_code>>
  {
    using std::tie;
    auto _ = std::ignore;

    auto results = co_await do_resolve();
    if (!results) {
      co_return unexpected { results.error() };
    }

    net::error_code ec;
    auto stream = net::tcp_stream(co_await net::this_coro::executor);

    stream.expires_after(request_timeout_);
    tie(ec, _) = co_await stream.async_connect(*results);
    if (ec) {
      log::debug("[{}] async_connect failed: {}", name(), ec.message());
      co_return unexpected { ec };
    }

    auto res = co_await do_send_recv(stream);
    if (!res) {
      co_return unexpected { res.error() };
    }

    stream.socket().shutdown(net::ip::tcp::socket::shutdown_send, ec);

    co_return res.value();
  }

#if defined(FITORIA_HAS_OPENSSL)
  auto do_session(net::ssl::context ssl_ctx) const
      -> lazy<expected<http_response, error_code>>
  {
    using std::tie;
    auto _ = std::ignore;

    auto results = co_await do_resolve();
    if (!results) {
      co_return unexpected { results.error() };
    }

    net::error_code ec;
    auto stream = net::ssl_stream(co_await net::this_coro::executor, ssl_ctx);

    // Set SNI Hostname (many hosts need this to handshake successfully)
    if (!SSL_set_tlsext_host_name(stream.native_handle(),
                                  resource_->host.c_str())) {
      co_return unexpected { net::error_code(
          static_cast<int>(::ERR_get_error()),
          net::error::get_ssl_category()) };
    }

    net::get_lowest_layer(stream).expires_after(request_timeout_);
    tie(ec, _) = co_await net::get_lowest_layer(stream).async_connect(*results);
    if (ec) {
      log::debug("[{}] async_connect failed: {}", name(), ec.message());
      co_return unexpected { ec };
    }

    net::get_lowest_layer(stream).expires_after(request_timeout_);
    tie(ec) = co_await stream.async_handshake(net::ssl::stream_base::client);
    if (ec) {
      log::debug("[{}] async_handshake failed: {}", name(), ec.message());
      co_return unexpected { ec };
    }

    auto res = co_await do_send_recv(stream);
    if (!res) {
      co_return unexpected { res.error() };
    }

    stream.shutdown(ec);
    if (ec == net::error::eof || ec == net::ssl::error::stream_truncated) {
      ec.clear();
    }
    if (ec) {
      co_return unexpected { ec };
    }

    co_return res.value();
  }
#endif

  template <typename Stream>
  auto do_send_recv(Stream& stream) const -> lazy<
      expected<boost::beast::http::response<boost::beast::http::string_body>,
               error_code>>
  {
    using boost::beast::http::response;
    using boost::beast::http::string_body;
    using std::tie;
    auto _ = std::ignore;

    if (body_ && body_->is_chunked()) {
      if (auto res = co_await do_send_req_with_chunk_body(stream); !res) {
        co_return unexpected { res.error() };
      } else if (*res) {
        co_return **res;
      }
    } else {
      if (auto res = co_await do_send_req_with_vector_body(stream); !res) {
        co_return unexpected { res.error() };
      } else if (*res) {
        co_return **res;
      }
    }

    net::error_code ec;
    net::flat_buffer buffer;

    response<string_body> res;
    net::get_lowest_layer(stream).expires_after(request_timeout_);
    tie(ec, _) = co_await async_read(stream, buffer, res);
    if (ec) {
      log::debug("[{}] async_read failed: {}", name(), ec.message());
      co_return unexpected { ec };
    }

    co_return res;
  }

  template <typename Stream>
  auto do_send_req_with_vector_body(Stream& stream) const -> lazy<expected<
      optional<boost::beast::http::response<boost::beast::http::string_body>>,
      error_code>>
  {
    using boost::beast::http::request;
    using boost::beast::http::request_serializer;
    using boost::beast::http::response;
    using boost::beast::http::string_body;
    using boost::beast::http::vector_body;
    using std::tie;
    auto _ = std::ignore;

    bool use_expect = fields_.get(http::field::expect) == "100-continue";

    request<vector_body<std::byte>> req(
        method_, get_encoded_target(resource_->path, query_.to_string()), 11);
    prepare_fields(req, fields_, use_expect);
    req.set(http::field::host, resource_->host);
    if (body_) {
      auto data = co_await web::async_read_all<std::vector<std::byte>>(
          any_async_readable_stream { *body_ });
      if (data) {
        if (!*data) {
          co_return unexpected { (*data).error() };
        }
        req.body() = std::move(**data);
      }
    }
    req.prepare_payload();

    net::error_code ec;

    auto req_serializer = request_serializer<vector_body<std::byte>>(req);
    net::get_lowest_layer(stream).expires_after(request_timeout_);
    tie(ec, _) = co_await async_write_header(stream, req_serializer);
    if (ec) {
      log::debug("[{}] async_write_header failed: {}", name(), ec.message());
      co_return unexpected { ec };
    }

    if (use_expect) {
      net::flat_buffer buffer;
      response<string_body> res;
      net::get_lowest_layer(stream).expires_after(request_timeout_);
      tie(ec, _) = co_await async_read(stream, buffer, res);
      if (ec && ec != boost::beast::error::timeout) {
        log::debug("[{}] async_read failed: {}", name(), ec.message());
        co_return unexpected { ec };
      }
      if (!ec && res.result() != http::status::continue_) {
        co_return res;
      }
    }

    net::get_lowest_layer(stream).expires_after(request_timeout_);
    tie(ec, _) = co_await async_write(stream, req_serializer);
    if (ec) {
      log::debug("[{}] async_write failed: {}", name(), ec.message());
      co_return unexpected { ec };
    }

    co_return nullopt;
  }

  template <typename Stream>
  auto do_send_req_with_chunk_body(Stream& stream) const -> lazy<expected<
      optional<boost::beast::http::response<boost::beast::http::string_body>>,
      error_code>>
  {
    using boost::beast::http::empty_body;
    using boost::beast::http::request;
    using boost::beast::http::request_serializer;
    using boost::beast::http::response;
    using boost::beast::http::string_body;
    using std::tie;
    auto _ = std::ignore;

    bool use_expect = fields_.get(http::field::expect) == "100-continue";

    request<empty_body> req(
        method_, get_encoded_target(resource_->path, query_.to_string()), 11);
    prepare_fields(req, fields_, use_expect);
    req.set(http::field::host, resource_->host);
    req.chunked(true);

    net::error_code ec;

    auto req_serializer = request_serializer<empty_body>(req);
    net::get_lowest_layer(stream).expires_after(request_timeout_);
    tie(ec, _) = co_await async_write_header(stream, req_serializer);
    if (ec) {
      log::debug("[{}] async_write_header failed: {}", name(), ec.message());
      co_return unexpected { ec };
    }

    if (use_expect) {
      net::flat_buffer buffer;
      response<string_body> res;
      net::get_lowest_layer(stream).expires_after(request_timeout_);
      tie(ec, _) = co_await async_read(stream, buffer, res);
      if (ec && ec != boost::beast::error::timeout) {
        log::debug("[{}] async_read failed: {}", name(), ec.message());
        co_return unexpected { ec };
      }
      if (!ec && res.result() != http::status::continue_) {
        co_return res;
      }
    }

    if (auto res = co_await web::async_write_each_chunk(
            stream, any_async_readable_stream { *body_ }, request_timeout_);
        !res) {
      log::debug(
          "[{}] async_write_each_chunk failed: {}", name(), ec.message());
      co_return unexpected { res.error() };
    }

    co_return nullopt;
  }

  static std::string get_encoded_target(std::string_view path,
                                        std::string_view query_string)
  {
    boost::urls::url url;
    url.set_path(path);
    url.set_query(query_string);
    return std::string(url.encoded_target());
  }

  template <bool IsRequest, class Body, class Fields>
  static void
  prepare_fields(boost::beast::http::message<IsRequest, Body, Fields>& req,
                 const http_fields& fields,
                 bool use_expect)
  {
    for (auto& [name, value] : fields) {
      if (name != to_string(http::field::expect) || use_expect) {
        req.insert(name, value);
      }
    }
  }

  expected<resource, error_code> resource_;
  query_map query_;
  http::verb method_ = http::verb::unknown;
  http_fields fields_;
  optional<any_async_readable_stream> body_;
  std::chrono::milliseconds request_timeout_ = std::chrono::seconds(5);
};
}

FITORIA_NAMESPACE_END

#endif
