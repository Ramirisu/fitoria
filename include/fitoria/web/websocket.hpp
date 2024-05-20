//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_WEB_WEBSOCKET_HPP
#define FITORIA_WEB_WEBSOCKET_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/core/buffer.hpp>
#include <fitoria/core/net.hpp>
#include <fitoria/core/optional.hpp>

#include <fitoria/web/error.hpp>
#include <fitoria/web/from_request.hpp>
#include <fitoria/web/response.hpp>

#include <functional>
#include <variant>

FITORIA_NAMESPACE_BEGIN

namespace web {

class http_server;

/// @verbatim embed:rst:leading-slashes
///
/// Extractor for upgrading the HTTP request to websocket protocol.
///
/// @endverbatim
class websocket {
  friend class http_server;
  friend class context;

  using duration_type = std::chrono::steady_clock::duration;
#if defined(FITORIA_HAS_OPENSSL)
  using stream_type = std::variant<websocket_stream<shared_tcp_stream>,
                                   websocket_stream<shared_ssl_stream>>;
#else
  using stream_type = std::variant<websocket_stream<shared_tcp_stream>>;
#endif

public:
  class context;

  enum class close_code : std::uint16_t {
    normal = 1000,
    going_away = 1001,
    protocol_error = 1002,
    unknown_data = 1003,
    bad_payload = 1007,
    policy_error = 1008,
    too_big = 1009,
    needs_extension = 1010,
    internal_error = 1011,
  };

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Wrapper type indicating that a ``text`` message is received.
  ///
  /// @endverbatim
  struct text_t {
    std::string value;
  };

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Wrapper type indicating that a ``binary`` message is received.
  ///
  /// @endverbatim
  struct binary_t {
    std::vector<std::byte> value;
  };

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Wrapper type indicating that a close control frame is received.
  ///
  /// @endverbatim
  struct close_t { };

  using message_type = std::variant<text_t, binary_t, close_t>;
  using callback_type = std::function<awaitable<void>(context&)>;

  class impl_type {
  public:
    impl_type(shared_tcp_stream stream)
        : stream_(std::in_place_index<0>, std::move(stream))
    {
    }

#if defined(FITORIA_HAS_OPENSSL)
    impl_type(shared_ssl_stream stream)
        : stream_(std::in_place_index<1>, std::move(stream))
    {
    }
#endif

    void set_option(boost::beast::websocket::stream_base::decorator d)
    {
      std::visit([&](auto& stream) { stream.set_option(std::move(d)); },
                 stream_);
    }

    void set_handshake_timeout(optional<duration_type> timeout) noexcept
    {
      timeout_.handshake_timeout
          = timeout.value_or(boost::beast::websocket::stream_base::none());
    }

    void set_idle_timeout(optional<duration_type> timeout) noexcept
    {
      timeout_.idle_timeout
          = timeout.value_or(boost::beast::websocket::stream_base::none());
    }

    void set_keep_alive_pings(bool enabled) noexcept
    {
      timeout_.keep_alive_pings = enabled;
    }

    template <typename Body>
    auto async_accept(boost::beast::http::request<Body>& req)
        -> awaitable<expected<void, std::error_code>>
    {
      co_return co_await std::visit(
          [&](auto& stream)
              -> awaitable<expected<void, boost::system::error_code>> {
            stream.set_option(timeout_);
            return stream.async_accept(req, use_awaitable);
          },
          stream_);
    }

    auto async_ping() -> awaitable<expected<void, std::error_code>>
    {
      co_return co_await std::visit(
          [](auto& stream)
              -> awaitable<expected<void, boost::system::error_code>> {
            return stream.async_ping({}, use_awaitable);
          },
          stream_);
    }

    auto async_read() -> awaitable<expected<message_type, std::error_code>>
    {
      auto buffer = dynamic_buffer<std::vector<std::byte>>();
      auto result = co_await std::visit(
          [&](auto& stream)
              -> awaitable<expected<std::size_t, boost::system::error_code>> {
            return stream.async_read(buffer, use_awaitable);
          },
          stream_);

      if (result) {
        if (is_text()) {
          co_return message_type(text_t { std::string(
              (char*)buffer.cdata().data(),
              (char*)buffer.cdata().data() + buffer.cdata().size()) });
        } else {
          co_return message_type(binary_t { buffer.release() });
        }
      } else {
        if (result.error() == boost::beast::websocket::error::closed) {
          co_return message_type(close_t {});
        } else {
          co_return unexpected { result.error() };
        }
      }
    }

    auto async_write_text(net::const_buffer buffer)
        -> awaitable<expected<void, std::error_code>>
    {
      co_return co_await std::visit(
          [&](auto& stream)
              -> awaitable<expected<std::size_t, boost::system::error_code>> {
            stream.text(true);
            return stream.async_write(buffer, use_awaitable);
          },
          stream_);
    }

    auto async_write_binary(net::const_buffer buffer)
        -> awaitable<expected<void, std::error_code>>
    {
      co_return co_await std::visit(
          [&](auto& stream)
              -> awaitable<expected<std::size_t, boost::system::error_code>> {
            stream.binary(true);
            return stream.async_write(buffer, use_awaitable);
          },
          stream_);
    }

    auto
    async_close(close_code code) -> awaitable<expected<void, std::error_code>>
    {
      co_return co_await std::visit(
          [&](auto& stream)
              -> awaitable<expected<void, boost::system::error_code>> {
            return stream.async_close(to_underlying(code), use_awaitable);
          },
          stream_);
    }

    void set_callback(callback_type callback)
    {
      callback_ = std::move(callback);
    }

    auto callback() noexcept -> callback_type&
    {
      FITORIA_ASSERT(callback_);
      return callback_;
    }

    void set_response(response res)
    {
      response_.emplace(std::move(res));
    }

    auto release_response() -> response
    {
      FITORIA_ASSERT(response_);
      auto res = std::move(*response_);
      response_.reset();
      return res;
    }

  private:
    auto is_text() const noexcept -> bool
    {
      return std::visit([](auto& stream) { return stream.got_text(); },
                        stream_);
    }

    stream_type stream_;
    callback_type callback_;
    optional<response> response_;
    boost::beast::websocket::stream_base::timeout timeout_
        = boost::beast::websocket::stream_base::timeout::suggested(
            boost::beast::role_type::server);
  };

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Provides APIs for interacting with the websocket protocol.
  ///
  /// @endverbatim
  class context {
    friend class websocket;

  public:
    /// @verbatim embed:rst:leading-slashes
    ///
    /// Send a websocket ping control frame asynchronously.
    ///
    /// @endverbatim
    auto async_ping() -> awaitable<expected<void, std::error_code>>
    {
      return ws_.impl_->async_ping();
    }

    /// @verbatim embed:rst:leading-slashes
    ///
    /// Read a complete message asynchronously.
    ///
    /// @endverbatim
    auto async_read() -> awaitable<expected<message_type, std::error_code>>
    {
      return ws_.impl_->async_read();
    }

    /// @verbatim embed:rst:leading-slashes
    ///
    /// Write a complete ``text`` message asynchronously.
    ///
    /// @endverbatim
    template <typename T, std::size_t N>
    auto async_write_text(std::span<T, N> buffer)
        -> awaitable<expected<void, std::error_code>>
    {
      return ws_.impl_->async_write_text(as_const_buffer(buffer));
    }

    /// @verbatim embed:rst:leading-slashes
    ///
    /// Write a complete ``text`` message asynchronously.
    ///
    /// @endverbatim
    auto async_write_text(std::string_view text)
        -> awaitable<expected<void, std::error_code>>
    {
      return async_write_text(std::span { text.data(), text.size() });
    }

    /// @verbatim embed:rst:leading-slashes
    ///
    /// Write a complete ``binary`` message asynchronously.
    ///
    /// @endverbatim
    template <typename T, std::size_t N>
    auto async_write_binary(std::span<T, N> buffer)
        -> awaitable<expected<void, std::error_code>>
    {
      return ws_.impl_->async_write_binary(as_const_buffer(buffer));
    }

    /// @verbatim embed:rst:leading-slashes
    ///
    /// Send a websocket close control frame asynchronously.
    ///
    /// @endverbatim
    auto
    async_close(close_code code) -> awaitable<expected<void, std::error_code>>
    {
      return ws_.impl_->async_close(code);
    }

  private:
    context(websocket& ws)
        : ws_(ws)
    {
    }

    websocket& ws_;
  };

  websocket(shared_tcp_stream stream)
      : impl_(std::make_shared<impl_type>(std::move(stream)))
  {
  }

#if defined(FITORIA_HAS_OPENSSL)
  websocket(shared_ssl_stream stream)
      : impl_(std::make_shared<impl_type>(std::move(stream)))
  {
  }
#endif

  auto set_handler(callback_type callback) -> response
  {
    impl_->set_callback(std::move(callback));
    return impl_->release_response();
  }

  void set_handshake_timeout(optional<duration_type> timeout) noexcept
  {
    impl_->set_handshake_timeout(timeout);
  }

  void set_idle_timeout(optional<duration_type> timeout) noexcept
  {
    impl_->set_idle_timeout(timeout);
  }

  void set_keep_alive_pings(bool enabled) noexcept
  {
    impl_->set_keep_alive_pings(enabled);
  }

  friend auto tag_invoke(from_request_t<websocket>, request& req)
      -> awaitable<expected<websocket, std::error_code>>
  {
    if (auto ws = req.state<websocket>(); ws) {
      ws->build_response(req);
      co_return *ws;
    }

    co_return unexpected { make_error_code(error::not_upgrade) };
  }

private:
  template <typename Body>
  auto run(boost::beast::http::request<Body>& req)
      -> awaitable<expected<void, std::error_code>>
  {
    namespace ws = boost::beast::websocket;

    if (auto res = impl_->release_response();
        !std::holds_alternative<any_body::null>(res.body().size())) {
      auto body
          = co_await async_read_until_eof<std::string>(res.body().stream());
      if (!body) {
        co_return unexpected { body.error() };
      }
      impl_->set_option(ws::stream_base::decorator(
          [&res, body = std::move(body)](ws::response_type& r) {
            for (auto& field : res.header()) {
              r.set(field.name(), field.value());
            }
            r.body() = std::move(*body);
          }));
    }

    if (auto result = co_await impl_->async_accept(req); !result) {
      co_return unexpected { result.error() };
    }

    auto ctx = context(*this);
    co_await impl_->callback()(ctx);
    co_return expected<void, std::error_code>();
  }

  // boost/beast/websocket/impl/accept.hpp
  void build_response(const request& req)
  {
    namespace ws = boost::beast::websocket;

    auto builder = response::switching_protocols();

    auto make_bad_request = [&, this](std::error_code ec) {
      builder.set_status_code(http::status::bad_request);
      impl_->set_response(builder.set_body(ec.message()));
    };

    if (req.version() != http::version::v1_1) {
      make_bad_request(make_error_code(ws::error::bad_http_version));
      return;
    }
    if (req.method() != http::verb::get) {
      make_bad_request(make_error_code(ws::error::bad_method));
      return;
    }
    if (!req.header().contains(http::field::host)) {
      make_bad_request(make_error_code(ws::error::no_host));
      return;
    }
    if (auto connection = req.header().get(http::field::connection);
        !connection) {
      make_bad_request(make_error_code(ws::error::no_connection));
      return;
    } else if (!boost::beast::http::token_list { *connection }.exists(
                   "upgrade")) {
      make_bad_request(make_error_code(ws::error::no_connection_upgrade));
      return;
    }
    if (auto upgrade = req.header().get(http::field::upgrade); !upgrade) {
      make_bad_request(make_error_code(ws::error::no_upgrade));
      return;
    } else if (!boost::beast::http::token_list { *upgrade }.exists(
                   "websocket")) {
      make_bad_request(make_error_code(ws::error::no_upgrade_websocket));
      return;
    }
    if (auto key = req.header().get(http::field::sec_websocket_key); !key) {
      make_bad_request(make_error_code(ws::error::no_sec_key));
      return;
    } else if (key->size() > ws::detail::sec_ws_key_type::static_capacity) {
      make_bad_request(make_error_code(ws::error::bad_sec_key));
      return;
    }
    if (auto ver = req.header().get(http::field::sec_websocket_version); !ver) {
      make_bad_request(make_error_code(ws::error::no_sec_version));
      return;
    } else if (*ver != "13") {
      builder.set_status_code(http::status::upgrade_required);
      builder.set_header(http::field::sec_websocket_version, "13");
      impl_->set_response(builder.set_body(
          make_error_code(ws::error::bad_sec_version).message()));
    }

    builder.set_header(http::field::upgrade, "websocket");
    builder.set_header(http::field::connection, "Upgrade");
    {
      ws::detail::sec_ws_accept_type acc;
      ws::detail::make_sec_ws_accept(
          acc, *req.header().get(http::field::sec_websocket_key));
      builder.set_header(http::field::sec_websocket_accept,
                         boost::beast::to_string_view(acc));
    }

    impl_->set_response(builder.build());
  }

  void set_response(response res)
  {
    impl_->set_response(std::move(res));
  }

  std::shared_ptr<impl_type> impl_;
};

}

FITORIA_NAMESPACE_END

#endif
