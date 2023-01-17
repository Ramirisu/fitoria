//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include <fitoria/core/config.hpp>

#include <fitoria/core/expected.hpp>
#include <fitoria/core/net.hpp>
#include <fitoria/core/url.hpp>

#include <fitoria/log/log.hpp>

#include <fitoria/web/error.hpp>
#include <fitoria/web/http/http.hpp>
#include <fitoria/web/http_header.hpp>
#include <fitoria/web/http_response.hpp>

FITORIA_NAMESPACE_BEGIN

class http_client {
  struct location {
    std::string host;
    std::uint16_t port;
    std::string target;
  };

public:
  http_client(std::string host, std::uint16_t port, std::string target)
      : host_(std::move(host))
      , port_(port)
      , target_(std::move(target))
  {
  }

  static expected<http_client, error_code> from_url(std::string url)
  {
    return parse_uri(std::move(url)).transform([](auto&& loc) {
      return http_client(std::move(loc.host), loc.port, std::move(loc.target));
    });
  }

  const std::string& host() const noexcept
  {
    return host_;
  }

  http_client& set_host(std::string host)
  {
    host_ = std::move(host);
    return *this;
  }

  const std::uint16_t& port() const noexcept
  {
    return port_;
  }

  http_client& set_port(std::uint16_t port)
  {
    port_ = port;
    return *this;
  }

  const std::string& target() const noexcept
  {
    return target_;
  }

  http_client& set_target(std::string target)
  {
    target_ = std::move(target);
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

  http_header& headers() noexcept
  {
    return header_;
  }

  const http_header& headers() const noexcept
  {
    return header_;
  }

  http_client& set_header(http::field name, std::string value)
  {
    header_.set(name, value);
    return *this;
  }

  http_client& set_header(std::string name, std::string value)
  {
    header_.set(name, value);
    return *this;
  }

  const std::string& body() const noexcept
  {
    return body_;
  }

  http_client& set_body(std::string body)
  {
    body_ = std::move(body);
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
    net::io_context ioc;
    auto fut = net::co_spawn(ioc, do_session(), net::use_future);
    ioc.run();
    return fut.get();
  }

#if defined(FITORIA_HAS_OPENSSL)
  auto send(net::ssl::context ssl_ctx) const
      -> expected<http_response, error_code>
  {
    net::io_context ioc;
    auto fut
        = net::co_spawn(ioc, do_session(std::move(ssl_ctx)), net::use_future);
    ioc.run();
    return fut.get();
  }
#endif

  auto async_send() const -> net::awaitable<expected<http_response, error_code>>
  {
    co_return co_await do_session();
  }

#if defined(FITORIA_HAS_OPENSSL)
  auto async_send(net::ssl::context ssl_ctx) const
      -> net::awaitable<expected<http_response, error_code>>
  {
    co_return co_await do_session(std::move(ssl_ctx));
  }
#endif

  const char* name() const noexcept
  {
    return "fitoria.http_client";
  }

private:
  static expected<location, error_code> parse_uri(std::string url)
  {
    auto res = urls::parse_uri(url);
    if (!res) {
      return unexpected { res.error() };
    }

    auto port_number = res->port_number();
    if (port_number == 0) {
      switch (res->scheme_id()) {
      case urls::scheme::https:
        port_number = 443;
        break;
      case urls::scheme::http:
      default:
        port_number = 80;
        break;
      }
    }

    return location { std::string(res->encoded_host()), port_number,
                      std::string(res->encoded_target()) };
  }

  auto do_resolve() const
      -> net::awaitable<expected<net::resolver_results, error_code>>
  {
    auto resolver = net::resolver(co_await net::this_coro::executor);
    auto [ec, results]
        = co_await resolver.async_resolve(host_, std::to_string(port_));
    if (ec) {
      log::debug("[{}] async_resolve failed: {}", name(), ec.message());
      co_return unexpected { ec };
    }

    co_return results;
  }

  auto do_session() const -> net::awaitable<expected<http_response, error_code>>
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

    auto res = co_await do_session_impl(stream);
    if (!res) {
      co_return unexpected { res.error() };
    }

    stream.socket().shutdown(net::ip::tcp::socket::shutdown_send, ec);

    co_return res.value();
  }

#if defined(FITORIA_HAS_OPENSSL)
  auto do_session(net::ssl::context ssl_ctx) const
      -> net::awaitable<expected<http_response, error_code>>
  {
    using std::tie;
    auto _ = std::ignore;

    auto results = co_await do_resolve();
    if (!results) {
      co_return unexpected { results.error() };
    }

    net::error_code ec;
    auto stream = net::ssl_stream(co_await net::this_coro::executor, ssl_ctx);

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

    auto res = co_await do_session_impl(stream);
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
  auto do_session_impl(Stream& stream) const
      -> net::awaitable<expected<http::response<http::string_body>, error_code>>
  {
    using std::tie;
    auto _ = std::ignore;

    bool use_expect_100_cont
        = header_.get(http::field::expect) == "100-continue" && !body_.empty();

    http::request<http::string_body> req { method_, target_, 11 };
    for (auto& [name, value] : header_) {
      if (name != to_string(http::field::expect) || use_expect_100_cont) {
        req.set(name, value);
      }
    }
    req.set(http::field::host, host_);
    req.body() = body_;
    req.prepare_payload();

    net::error_code ec;
    net::flat_buffer buffer;

    http::request_serializer<http::string_body> req_serializer(req);
    if (use_expect_100_cont) {
      net::get_lowest_layer(stream).expires_after(request_timeout_);
      tie(ec, _) = co_await http::async_write_header(stream, req_serializer);
      if (ec) {
        log::debug("[{}] async_write_header failed: {}", name(), ec.message());
        co_return unexpected { ec };
      }

      http::response<http::string_body> res;
      net::get_lowest_layer(stream).expires_after(request_timeout_);
      tie(ec, _) = co_await http::async_read(stream, buffer, res);
      if (ec && ec != boost::beast::error::timeout) {
        log::debug("[{}] async_read failed: {}", name(), ec.message());
        co_return unexpected { ec };
      }
      if (res.result() != http::status::continue_) {
        co_return res;
      }
    }

    net::get_lowest_layer(stream).expires_after(request_timeout_);
    tie(ec, _) = co_await http::async_write(stream, req_serializer);
    if (ec) {
      log::debug("[{}] async_write failed: {}", name(), ec.message());
      co_return unexpected { ec };
    }

    http::response<http::string_body> res;

    net::get_lowest_layer(stream).expires_after(request_timeout_);
    tie(ec, _) = co_await http::async_read(stream, buffer, res);
    if (ec) {
      log::debug("[{}] async_read failed: {}", name(), ec.message());
      co_return unexpected { ec };
    }

    co_return res;
  }

  std::string host_;
  std::uint16_t port_;
  std::string target_;
  http::verb method_ = http::verb::unknown;
  http_header header_;
  std::string body_;
  std::chrono::milliseconds request_timeout_ = std::chrono::seconds(5);
};

FITORIA_NAMESPACE_END
