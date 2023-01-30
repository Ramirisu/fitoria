//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef FITORIA_WEB_HTTP_CLIENT_HPP
#define FITORIA_WEB_HTTP_CLIENT_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/core/expected.hpp>
#include <fitoria/core/net.hpp>
#include <fitoria/core/url.hpp>

#include <fitoria/log/log.hpp>

#include <fitoria/web/error.hpp>
#include <fitoria/web/http/http.hpp>
#include <fitoria/web/http_fields.hpp>
#include <fitoria/web/http_response.hpp>
#include <fitoria/web/query_map.hpp>

FITORIA_NAMESPACE_BEGIN

class http_client {
public:
  struct resource {
    std::string host;
    std::uint16_t port;
    std::string path;

    static expected<resource, error_code> parse_uri(std::string_view url)
    {
      auto encoded_url = encode_whitespace(url); // FIXME:
      auto res = urls::parse_uri(encoded_url);
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

      return resource { std::string(res->host()), port_number,
                        std::string(res->path()) };
    }

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
  };

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
    fields_.set(name, value);
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

  auto send(expected<resource, error_code> rc) const
      -> expected<http_response, error_code>
  {
    if (!rc) {
      return unexpected { rc.error() };
    }

    net::io_context ioc;
    auto fut = net::co_spawn(ioc, do_session(std::move(rc.value())),
                             net::use_future);
    ioc.run();
    return fut.get();
  }

#if defined(FITORIA_HAS_OPENSSL)
  auto send(expected<resource, error_code> rc, net::ssl::context ssl_ctx) const
      -> expected<http_response, error_code>
  {
    if (!rc) {
      return unexpected { rc.error() };
    }

    net::io_context ioc;
    auto fut = net::co_spawn(
        ioc, do_session(std::move(rc.value()), std::move(ssl_ctx)),
        net::use_future);
    ioc.run();
    return fut.get();
  }
#endif

  auto async_send(expected<resource, error_code> rc) const
      -> net::awaitable<expected<http_response, error_code>>
  {
    if (!rc) {
      co_return unexpected { rc.error() };
    }

    co_return co_await do_session(std::move(rc.value()));
  }

#if defined(FITORIA_HAS_OPENSSL)
  auto async_send(expected<resource, error_code> rc,
                  net::ssl::context ssl_ctx) const
      -> net::awaitable<expected<http_response, error_code>>
  {
    if (!rc) {
      co_return unexpected { rc.error() };
    }

    co_return co_await do_session(std::move(rc.value()), std::move(ssl_ctx));
  }
#endif

  const char* name() const noexcept
  {
    return "fitoria.http_client";
  }

private:
  auto do_resolve(const resource& loc) const
      -> net::awaitable<expected<net::resolver_results, error_code>>
  {
    auto resolver = net::resolver(co_await net::this_coro::executor);
    auto [ec, results]
        = co_await resolver.async_resolve(loc.host, std::to_string(loc.port));
    if (ec) {
      log::debug("[{}] async_resolve failed: {}", name(), ec.message());
      co_return unexpected { ec };
    }

    co_return results;
  }

  auto do_session(const resource& loc) const
      -> net::awaitable<expected<http_response, error_code>>
  {
    using std::tie;
    auto _ = std::ignore;

    auto results = co_await do_resolve(loc);
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

    auto res = co_await do_send_recv(loc, stream);
    if (!res) {
      co_return unexpected { res.error() };
    }

    stream.socket().shutdown(net::ip::tcp::socket::shutdown_send, ec);

    co_return res.value();
  }

#if defined(FITORIA_HAS_OPENSSL)
  auto do_session(const resource& loc, net::ssl::context ssl_ctx) const
      -> net::awaitable<expected<http_response, error_code>>
  {
    using std::tie;
    auto _ = std::ignore;

    auto results = co_await do_resolve(loc);
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

    auto res = co_await do_send_recv(loc, stream);
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
  auto
  do_send_recv(const resource& loc, Stream& stream) const -> net::awaitable<
      expected<http::detail::response<http::detail::string_body>, error_code>>
  {
    using std::tie;
    auto _ = std::ignore;

    bool use_expect_100_cont
        = fields_.get(http::field::expect) == "100-continue" && !body_.empty();

    http::detail::request<http::detail::string_body> req {
      method_, get_encoded_target(loc.path, query_.to_string()), 11
    };
    for (auto& [name, value] : fields_) {
      if (name != to_string(http::field::expect) || use_expect_100_cont) {
        req.set(name, value);
      }
    }
    req.set(http::field::host, loc.host);
    req.body() = body_;
    req.prepare_payload();

    net::error_code ec;
    net::flat_buffer buffer;

    http::detail::request_serializer<http::detail::string_body> req_serializer(
        req);
    if (use_expect_100_cont) {
      net::get_lowest_layer(stream).expires_after(request_timeout_);
      tie(ec, _)
          = co_await http::detail::async_write_header(stream, req_serializer);
      if (ec) {
        log::debug("[{}] async_write_header failed: {}", name(), ec.message());
        co_return unexpected { ec };
      }

      http::detail::response<http::detail::string_body> res;
      net::get_lowest_layer(stream).expires_after(request_timeout_);
      tie(ec, _) = co_await http::detail::async_read(stream, buffer, res);
      if (ec && ec != boost::beast::error::timeout) {
        log::debug("[{}] async_read failed: {}", name(), ec.message());
        co_return unexpected { ec };
      }
      if (res.result() != http::status::continue_) {
        co_return res;
      }
    }

    net::get_lowest_layer(stream).expires_after(request_timeout_);
    tie(ec, _) = co_await http::detail::async_write(stream, req_serializer);
    if (ec) {
      log::debug("[{}] async_write failed: {}", name(), ec.message());
      co_return unexpected { ec };
    }

    http::detail::response<http::detail::string_body> res;

    net::get_lowest_layer(stream).expires_after(request_timeout_);
    tie(ec, _) = co_await http::detail::async_read(stream, buffer, res);
    if (ec) {
      log::debug("[{}] async_read failed: {}", name(), ec.message());
      co_return unexpected { ec };
    }

    co_return res;
  }

  static std::string get_encoded_target(std::string_view path,
                                        std::string_view query_string)
  {
    urls::url url;
    url.set_path(path);
    url.set_query(query_string);
    return std::string(url.encoded_target());
  }

  query_map query_;
  http::verb method_ = http::verb::unknown;
  http_fields fields_;
  std::string body_;
  std::chrono::milliseconds request_timeout_ = std::chrono::seconds(5);
};

FITORIA_NAMESPACE_END

#endif
