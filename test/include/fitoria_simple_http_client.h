//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include <fitoria/core/config.hpp>

#include <fitoria/core/http.hpp>
#include <fitoria/core/net.hpp>

#include <string>
#include <string_view>

FITORIA_NAMESPACE_BEGIN

class simple_http_client {
public:
  simple_http_client(std::string host, std::uint16_t port)
      : host_(std::move(host))
      , port_(port)
  {
  }

  simple_http_client& with(http::verb method)
  {
    method_ = method;
    return *this;
  }

  simple_http_client& with_target(std::string target)
  {
    target_ = std::move(target);
    return *this;
  }

  simple_http_client& with_field(http::field name, std::string_view value)
  {
    fields_.set(name, value);
    return *this;
  }

  simple_http_client& with_body(std::string body)
  {
    body_ = std::move(body);
    return *this;
  }

  simple_http_client& set_request_timeout(std::chrono::milliseconds timeout)
  {
    request_timeout_ = timeout;
    return *this;
  }

  auto send_request()
  {
    net::io_context ioc;
    auto future = net::co_spawn(ioc, do_session(), net::use_future);
    ioc.run();

    return future.get();
  }

#if defined(FITORIA_HAS_OPENSSL)
  auto send_request(net::ssl::context ssl_ctx)
  {
    net::io_context ioc;
    auto future
        = net::co_spawn(ioc, do_session(std::move(ssl_ctx)), net::use_future);
    ioc.run();

    return future.get();
  }
#endif

private:
  using resolver
      = net::use_awaitable_t<>::as_default_on_t<net::ip::tcp::resolver>;
  using tcp_stream
      = net::use_awaitable_t<>::as_default_on_t<boost::beast::tcp_stream>;
#if defined(FITORIA_HAS_OPENSSL)
  using ssl_stream = boost::beast::ssl_stream<tcp_stream>;
#endif

  net::awaitable<resolver> new_resolver()
  {
    co_return resolver(co_await net::this_coro::executor);
  }

  net::awaitable<http::response<http::string_body>> do_session()
  {
    auto resolver = co_await new_resolver();
    auto stream = tcp_stream(co_await net::this_coro::executor);

    const auto results
        = co_await resolver.async_resolve(host_, std::to_string(port_));

    stream.expires_after(request_timeout_);

    co_await stream.async_connect(results);

    auto resp = co_await do_session_impl(stream);

    net::error_code ec;
    stream.socket().shutdown(net::ip::tcp::socket::shutdown_send, ec);

    co_return resp;
  }

#if defined(FITORIA_HAS_OPENSSL)
  net::awaitable<http::response<http::string_body>>
  do_session(net::ssl::context ssl_ctx)
  {
    auto resolver = co_await new_resolver();
    auto stream = ssl_stream(co_await net::this_coro::executor, ssl_ctx);

    auto hostname = net::ip::host_name();
    SSL_set_tlsext_host_name(stream.native_handle(), hostname.c_str());

    const auto results
        = co_await resolver.async_resolve(host_, std::to_string(port_));

    net::get_lowest_layer(stream).expires_after(request_timeout_);

    co_await net::get_lowest_layer(stream).async_connect(results);

    net::get_lowest_layer(stream).expires_after(request_timeout_);

    co_await stream.async_handshake(net::ssl::stream_base::client);

    auto resp = co_await do_session_impl(stream);

    net::error_code ec;
    stream.shutdown(ec);
    if (ec == net::error::eof || ec == net::ssl::error::stream_truncated) {
      ec.clear();
    }
    if (ec) {
      throw net::system_error(ec);
    }

    co_return resp;
  }
#endif

  template <typename Stream>
  net::awaitable<http::response<http::string_body>>
  do_session_impl(Stream& stream)
  {
    http::request<http::string_body> req { method_, target_, 11 };
    for (const auto& f : fields_) {
      req.set(f.name(), f.value());
    }
    req.set(http::field::host, host_);
    req.body() = body_;
    req.prepare_payload();

    net::flat_buffer buffer;

    if (auto it = req.find(http::field::expect);
        it != req.end() && it->value() == "100-continue") {
      http::request_serializer<http::string_body> serializer(req);
      net::get_lowest_layer(stream).expires_after(request_timeout_);
      co_await http::async_write_header(stream, serializer);

      try {
        http::response<http::string_body> res;
        net::get_lowest_layer(stream).expires_after(request_timeout_);
        co_await http::async_read(stream, buffer, res);
        if (res.result() != http::status::continue_) {
          co_return res;
        }
      } catch (const net::system_error& ex) {
        if (ex.code() != boost::beast::error::timeout) {
          throw;
        }
      }

      net::get_lowest_layer(stream).expires_after(request_timeout_);
      co_await http::async_write(stream, serializer);
    } else {
      net::get_lowest_layer(stream).expires_after(request_timeout_);
      co_await http::async_write(stream, req);
    }

    http::response<http::string_body> res;

    net::get_lowest_layer(stream).expires_after(request_timeout_);
    co_await http::async_read(stream, buffer, res);

    co_return res;
  }

  std::string host_;
  std::uint16_t port_;
  http::verb method_ = http::verb::unknown;
  std::string target_;
  http::fields fields_;
  std::string body_;
  std::chrono::milliseconds request_timeout_ = std::chrono::seconds(5);
};

FITORIA_NAMESPACE_END
