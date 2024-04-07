//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_WEB_HTTP_SERVER_HPP
#define FITORIA_WEB_HTTP_SERVER_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/core/coroutine_concept.hpp>
#include <fitoria/core/net.hpp>
#include <fitoria/core/url.hpp>

#include <fitoria/log.hpp>

#include <fitoria/web/async_message_parser_stream.hpp>
#include <fitoria/web/async_write_each_chunk.hpp>
#include <fitoria/web/handler.hpp>
#include <fitoria/web/http_request.hpp>
#include <fitoria/web/http_response.hpp>
#include <fitoria/web/router.hpp>
#include <fitoria/web/scope.hpp>

#include <system_error>

FITORIA_NAMESPACE_BEGIN

namespace web {

class http_server_builder;

class http_server {
  friend class http_server_builder;

  using request_type = http_request&;
  using response_type = awaitable<http_response>;
  using router_type = router<request_type, response_type>;

  http_server(
      executor_type ex,
      router_type router,
      optional<int> max_listen_connections,
      optional<std::chrono::milliseconds> client_request_timeout,
      optional<std::chrono::milliseconds> tls_handshake_timeout
#if !FITORIA_NO_EXCEPTIONS
      ,
      optional<std::function<void(std::exception_ptr)>> exception_handler
#endif
      )
      : ex_(std::move(ex))
      , router_(std::move(router))
      , max_listen_connections_(max_listen_connections.value_or(
            static_cast<int>(net::socket_base::max_listen_connections)))
      , client_request_timeout_(
            client_request_timeout.value_or(std::chrono::seconds(5)))
      , tls_handshake_timeout_(
            tls_handshake_timeout.value_or(std::chrono::seconds(3)))
#if !FITORIA_NO_EXCEPTIONS
      , exception_handler_(
            exception_handler.value_or(default_exception_handler))
#endif
  {
  }

public:
  http_server(const http_server&) = delete;

  http_server& operator=(const http_server&) = delete;

  http_server(http_server&&) = delete;

  http_server& operator=(http_server&&) = delete;

  auto max_listen_connections() const noexcept -> int
  {
    return max_listen_connections_;
  }

  auto client_request_timeout() const noexcept -> std::chrono::milliseconds
  {
    return client_request_timeout_;
  }

  auto tls_handshake_timeout() const noexcept -> std::chrono::milliseconds
  {
    return tls_handshake_timeout_;
  }

  auto bind(std::string_view addr, std::uint16_t port) const
      -> expected<const http_server&, std::error_code>
  {
    auto acceptor = make_acceptor(
        net::ip::tcp::endpoint(net::ip::make_address(addr), port));
    if (!acceptor) {
      return unexpected { acceptor.error() };
    }

    net::co_spawn(ex_, do_listen(std::move(*acceptor)), exception_handler_);

    return expected<const http_server&, std::error_code>(*this);
  }

#if defined(FITORIA_HAS_OPENSSL)
  auto bind_ssl(std::string_view addr,
                std::uint16_t port,
                net::ssl::context ssl_ctx) const
      -> expected<const http_server&, std::error_code>
  {
    auto acceptor = make_acceptor(
        net::ip::tcp::endpoint(net::ip::make_address(addr), port));
    if (!acceptor) {
      return unexpected { acceptor.error() };
    }

    net::co_spawn(ex_,
                  do_listen(std::move(*acceptor), std::move(ssl_ctx)),
                  exception_handler_);

    return expected<const http_server&, std::error_code>(*this);
  }
#endif

  template <typename F>
    requires std::is_invocable_v<F, http_response>
      && co_awaitable<std::invoke_result_t<F, http_response>>
  void serve_request(std::string_view path, http_request req, F f) const
  {
    auto target = [&]() -> std::string {
      boost::urls::url url;
      url.set_path(path);
      url.set_query(req.query().to_string());
      return std::string(url.encoded_target());
    }();

    net::co_spawn(
        ex_,
        serve_request_impl(std::move(req), std::move(target), std::move(f)),
        net::detached);
  }

private:
  template <typename F>
  auto serve_request_impl(http_request req, std::string target, F f) const
      -> awaitable<void>
  {
    auto res = co_await do_handler(
        connection_info(
            net::ip::tcp::endpoint(net::ip::make_address("127.0.0.1"), 0),
            net::ip::tcp::endpoint(net::ip::make_address("127.0.0.1"), 0)),
        req.method(),
        target,
        std::move(req.fields()),
        std::move(req.body()));
    co_await f(std::move(res));
  }

  auto make_acceptor(net::ip::tcp::endpoint endpoint) const
      -> expected<socket_acceptor, std::error_code>
  {
    auto acceptor = socket_acceptor(ex_);

    boost::system::error_code ec;
    acceptor.open(endpoint.protocol(), ec);
    if (ec) {
      return unexpected { ec };
    }

    acceptor.set_option(net::socket_base::reuse_address(true), ec);
    if (ec) {
      return unexpected { ec };
    }

    acceptor.bind(endpoint, ec);
    if (ec) {
      return unexpected { ec };
    }

    acceptor.listen(max_listen_connections_, ec);
    if (ec) {
      return unexpected { ec };
    }

    return acceptor;
  }

  auto do_listen(socket_acceptor acceptor) const -> awaitable<void>
  {
    for (;;) {
      if (auto [ec, socket] = co_await acceptor.async_accept(use_awaitable);
          !ec) {
        net::co_spawn(
            ex_,
            do_session(std::make_shared<tcp_stream>(std::move(socket))),
            exception_handler_);
      }
    }
  }

#if defined(FITORIA_HAS_OPENSSL)
  auto do_listen(socket_acceptor acceptor, net::ssl::context ssl_ctx) const
      -> awaitable<void>
  {
    auto ssl_ctx_ptr = std::make_shared<net::ssl::context>(std::move(ssl_ctx));

    for (;;) {
      if (auto [ec, socket] = co_await acceptor.async_accept(use_awaitable);
          !ec) {
        net::co_spawn(ex_,
                      do_session(std::make_shared<safe_ssl_stream>(
                          std::move(socket), ssl_ctx_ptr)),
                      exception_handler_);
      }
    }
  }
#endif

  auto do_session(std::shared_ptr<tcp_stream> stream) const -> awaitable<void>
  {
    if (auto ec = co_await do_session_impl(stream); ec) {
      FITORIA_THROW_OR(std::system_error(ec), co_return);
    }

    boost::system::error_code ec;
    stream->socket().shutdown(net::ip::tcp::socket::shutdown_send, ec);
  }

#if defined(FITORIA_HAS_OPENSSL)
  auto do_session(std::shared_ptr<safe_ssl_stream> stream) const
      -> awaitable<void>
  {
    using boost::beast::get_lowest_layer;
    boost::system::error_code ec;

    get_lowest_layer(*stream).expires_after(tls_handshake_timeout_);
    std::tie(ec) = co_await stream->async_handshake(
        net::ssl::stream_base::server, use_awaitable);
    if (ec) {
      FITORIA_THROW_OR(std::system_error(ec), co_return);
    }

    if (ec = co_await do_session_impl(stream); ec) {
      FITORIA_THROW_OR(std::system_error(ec), co_return);
    }

    std::tie(ec) = co_await stream->async_shutdown(use_awaitable);
    if (ec) {
      FITORIA_THROW_OR(std::system_error(ec), co_return);
    }
  }
#endif

  template <typename Stream>
  auto do_session_impl(std::shared_ptr<Stream>& stream) const
      -> awaitable<std::error_code>
  {
    using boost::beast::flat_buffer;
    using boost::beast::get_lowest_layer;
    using boost::beast::http::buffer_body;
    using boost::beast::http::empty_body;
    using boost::beast::http::request_parser;
    using boost::beast::http::response;

    boost::system::error_code ec;

    for (;;) {
      flat_buffer buffer;
      auto parser = std::make_unique<request_parser<buffer_body>>();
      parser->body_limit(boost::none);

      get_lowest_layer(*stream).expires_after(client_request_timeout_);
      std::tie(ec, std::ignore)
          = co_await async_read_header(*stream, buffer, *parser, use_awaitable);
      if (ec) {
        co_return ec;
      }

      bool keep_alive = parser->get().keep_alive();
      auto method = parser->get().method();
      auto target = std::string(parser->get().target());
      auto fields = http_fields::from_impl(parser->get());

      if (auto it = parser->get().find(http::field::expect);
          it != parser->get().end() && it->value() == "100-continue") {
        std::tie(ec, std::ignore) = co_await async_write(
            *stream,
            response<empty_body>(http::status::continue_, 11),
            use_awaitable);
        if (ec) {
          co_return ec;
        }
      }

      auto res = co_await do_handler(
          connection_info(get_lowest_layer(*stream).socket().local_endpoint(),
                          get_lowest_layer(*stream).socket().remote_endpoint()),
          method,
          std::move(target),
          std::move(fields),
          async_message_parser_stream(
              std::move(buffer), stream, std::move(parser)));

      auto do_response = [&]() -> awaitable<expected<void, std::error_code>> {
        return res.body().size_hint()
            ? do_sized_response(stream, res, keep_alive)
            : do_chunked_response(stream, res, keep_alive);
      };
      if (auto exp = co_await do_response(); !exp) {
        co_return exp.error();
      }

      if (!keep_alive) {
        break;
      }
    }

    co_return ec;
  }

  auto do_handler(connection_info connection_info,
                  http::verb method,
                  std::string target,
                  http_fields fields,
                  any_async_readable_stream body) const
      -> awaitable<http_response>
  {
    auto req_url = boost::urls::parse_origin_form(target);
    if (!req_url) {
      co_return http_response(http::status::bad_request)
          .set_field(http::field::content_type,
                     http::fields::content_type::plaintext())
          .set_body("request target is invalid");
    }

    if (auto route = router_.try_find(method, req_url.value().path()); route) {
      auto request = http_request(
          std::move(connection_info),
          path_info(std::string(route->matcher().pattern()),
                    req_url->path(),
                    route->matcher().match(req_url->path()).value()),
          method,
          query_map::from(req_url->params()),
          std::move(fields),
          std::move(body),
          route->state_maps());
      co_return co_await route->operator()(request);
    }

    co_return http_response(http::status::not_found)
        .set_field(http::field::content_type,
                   http::fields::content_type::plaintext())
        .set_body("request path is not found");
  }

  template <typename Stream>
  auto do_sized_response(std::shared_ptr<Stream>& stream,
                         http_response& res,
                         bool keep_alive) const
      -> awaitable<expected<void, std::error_code>>
  {
    using boost::beast::get_lowest_layer;
    using boost::beast::http::response;
    using boost::beast::http::vector_body;

    auto r = response<vector_body<std::byte>>(res.status_code().value(), 11);
    res.fields().to_impl(r);
    r.keep_alive(keep_alive);
    if (auto data
        = co_await async_read_until_eof<std::vector<std::byte>>(res.body());
        data) {
      r.body() = std::move(*data);
      r.prepare_payload();
    } else if (data.error() == make_error_code(net::error::eof)) {
      r.prepare_payload();

      // boost.beast incorrectly handle some cases
      // https://datatracker.ietf.org/doc/html/rfc7230#section-3.3.2
      if (res.status_code().category() == http::status_class::informational
          || res.status_code().value() == http::status::no_content) {
        r.content_length(boost::none);
      }
    } else {
      co_return unexpected { data.error() };
    }

    if (auto [ec, _] = co_await async_write(*stream, r, use_awaitable); ec) {
      co_return unexpected { ec };
    }

    co_return expected<void, std::error_code>();
  }

  template <typename Stream>
  auto do_chunked_response(std::shared_ptr<Stream>& stream,
                           http_response& res,
                           bool keep_alive) const
      -> awaitable<expected<void, std::error_code>>
  {
    using boost::beast::get_lowest_layer;
    using boost::beast::http::empty_body;
    using boost::beast::http::response;
    using boost::beast::http::response_serializer;

    auto r = response<empty_body>(res.status_code().value(), 11);
    res.fields().to_impl(r);
    r.keep_alive(keep_alive);
    r.chunked(true);

    auto ser = response_serializer<empty_body>(r);

    if (auto [ec, _] = co_await async_write_header(*stream, ser, use_awaitable);
        ec) {
      co_return unexpected { ec };
    }

    co_return co_await async_write_each_chunk(*stream, res.body());
  }

#if !FITORIA_NO_EXCEPTIONS
  static void default_exception_handler(std::exception_ptr ptr)
  {
    if (ptr) {
      try {
        std::rethrow_exception(ptr);
      } catch (const std::exception& ex) {
        log::debug("[{}] exception: {}", name(), ex.what());
      }
    }
  }
#endif

  static const char* name() noexcept
  {
    return "fitoria.web.http_server";
  }

  executor_type ex_;
  router_type router_;
  int max_listen_connections_;
  std::chrono::milliseconds client_request_timeout_;
  std::chrono::milliseconds tls_handshake_timeout_;
#if !FITORIA_NO_EXCEPTIONS
  std::function<void(std::exception_ptr)> exception_handler_;
#else
  net::detached_t exception_handler_;
#endif
};

class http_server_builder {
  using request_type = http_request&;
  using response_type = awaitable<http_response>;
  using router_type = router<request_type, response_type>;

public:
  http_server_builder(const executor_type& ex)
      : ex_(ex)
  {
  }

  template <typename ExecutionContext>
    requires std::is_convertible_v<ExecutionContext&, net::execution_context&>
  http_server_builder(ExecutionContext& context)
      : ex_(context.get_executor())
  {
  }

  auto set_max_listen_connections(int num) & noexcept -> http_server_builder&
  {
    max_listen_connections_ = num;
    return *this;
  }

  auto set_max_listen_connections(int num) && noexcept -> http_server_builder&&
  {
    set_max_listen_connections(num);
    return std::move(*this);
  }

  auto set_client_request_timeout(std::chrono::milliseconds timeout) & noexcept
      -> http_server_builder&
  {
    client_request_timeout_ = timeout;
    return *this;
  }

  auto set_client_request_timeout(std::chrono::milliseconds timeout) && noexcept
      -> http_server_builder&&
  {
    set_client_request_timeout(timeout);
    return std::move(*this);
  }

  auto set_tls_handshake_timeout(std::chrono::milliseconds timeout) & noexcept
      -> http_server_builder&
  {
    tls_handshake_timeout_ = timeout;
    return *this;
  }

  auto set_tls_handshake_timeout(std::chrono::milliseconds timeout) && noexcept
      -> http_server_builder&&
  {
    set_tls_handshake_timeout(timeout);
    return std::move(*this);
  }

#if !FITORIA_NO_EXCEPTIONS
  template <typename F>
    requires std::invocable<F, std::exception_ptr>
  auto set_exception_handler(F&& f) & -> http_server_builder&
  {
    exception_handler_.emplace(std::forward<F>(f));
    return *this;
  }

  template <typename F>
    requires std::invocable<F, std::exception_ptr>
  auto set_exception_handler(F&& f) && -> http_server_builder&&
  {
    set_exception_handler(std::forward<F>(f));
    return std::move(*this);
  }
#endif

  template <basic_fixed_string RoutePath,
            typename... RouteServices,
            typename Handler>
  auto serve(route_impl<RoutePath, std::tuple<RouteServices...>, Handler>
                 route) & -> http_server_builder&
  {
    if (auto res = router_.try_insert(typename router_type::route_type(
            route.template build<request_type, response_type>(handler())));
        !res) {
      FITORIA_THROW_OR(std::system_error(res.error()), std::terminate());
    }

    return *this;
  }

  template <basic_fixed_string RoutePath,
            typename... RouteServices,
            typename Handler>
  auto serve(route_impl<RoutePath, std::tuple<RouteServices...>, Handler>
                 route) && -> http_server_builder&&
  {
    serve(std::move(route));

    return std::move(*this);
  }

  template <basic_fixed_string Path, typename... Services, typename... Routes>
  auto serve(scope_impl<Path, std::tuple<Services...>, std::tuple<Routes...>>
                 scope) & -> http_server_builder&
  {
    std::apply(
        [this](auto&&... routes) {
          (this->serve(std::forward<decltype(routes)>(routes)), ...);
        },
        scope.routes());

    return *this;
  }

  template <basic_fixed_string Path, typename... Services, typename... Routes>
  auto serve(scope_impl<Path, std::tuple<Services...>, std::tuple<Routes...>>
                 scope) && -> http_server_builder&&
  {
    serve(std::move(scope));

    return std::move(*this);
  }

  auto build() -> http_server
  {
    router_.optimize();

    return http_server(std::move(ex_),
                       std::move(router_),
                       max_listen_connections_,
                       client_request_timeout_,
                       tls_handshake_timeout_,
                       std::move(exception_handler_));
  }

private:
  executor_type ex_;
  router_type router_;
  optional<int> max_listen_connections_;
  optional<std::chrono::milliseconds> client_request_timeout_;
  optional<std::chrono::milliseconds> tls_handshake_timeout_;
#if !FITORIA_NO_EXCEPTIONS
  optional<std::function<void(std::exception_ptr)>> exception_handler_;
#else
  net::detached_t exception_handler_;
#endif
};

}

FITORIA_NAMESPACE_END

#endif
