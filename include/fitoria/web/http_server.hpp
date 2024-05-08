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
#include <fitoria/core/utility.hpp>

#include <fitoria/log.hpp>

#include <fitoria/web/detail/make_acceptor.hpp>

#include <fitoria/web/async_message_parser_stream.hpp>
#include <fitoria/web/async_write_chunks.hpp>
#include <fitoria/web/handler.hpp>
#include <fitoria/web/request.hpp>
#include <fitoria/web/response.hpp>
#include <fitoria/web/router.hpp>
#include <fitoria/web/scope.hpp>
#include <fitoria/web/test_response.hpp>

#include <system_error>

FITORIA_NAMESPACE_BEGIN

namespace web {

class http_server_builder;

class http_server {
  friend class http_server_builder;

  using request_type = request&;
  using response_type = awaitable<response>;
  using router_type = router<request_type, response_type>;
#if FITORIA_NO_EXCEPTIONS
  using exception_handler_t = net::detached_t;
#else
  using exception_handler_t = std::function<void(std::exception_ptr)>;
#endif

  http_server(executor_type ex,
              router_type router,
              optional<int> max_listen_connections,
              optional<std::chrono::milliseconds> client_request_timeout,
              optional<std::chrono::milliseconds> tls_handshake_timeout,
              optional<exception_handler_t> exception_handler)
      : ex_(std::move(ex))
      , router_(std::move(router))
      , max_listen_connections_(max_listen_connections.value_or(
            static_cast<int>(net::socket_base::max_listen_connections)))
      , client_request_timeout_(client_request_timeout)
      , tls_handshake_timeout_(tls_handshake_timeout)
      , exception_handler_(
            exception_handler.value_or(default_exception_handler))
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

  auto
  client_request_timeout() const noexcept -> optional<std::chrono::milliseconds>
  {
    return client_request_timeout_;
  }

  auto
  tls_handshake_timeout() const noexcept -> optional<std::chrono::milliseconds>
  {
    return tls_handshake_timeout_;
  }

  auto bind(std::string_view addr, std::uint16_t port) const
      -> expected<const http_server&, std::error_code>
  {
    auto acceptor
        = detail::make_acceptor(ex_, addr, port, max_listen_connections_);
    if (!acceptor) {
      return unexpected { acceptor.error() };
    }

    net::co_spawn(ex_, do_listen(std::move(*acceptor)), exception_handler_);

    return expected<const http_server&, std::error_code>(*this);
  }

#if defined(FITORIA_HAS_OPENSSL)
  auto bind_ssl(std::string_view addr,
                std::uint16_t port,
                net::ssl::context& ssl_ctx) const
      -> expected<const http_server&, std::error_code>
  {
    auto acceptor
        = detail::make_acceptor(ex_, addr, port, max_listen_connections_);
    if (!acceptor) {
      return unexpected { acceptor.error() };
    }

    net::co_spawn(
        ex_, do_listen(std::move(*acceptor), ssl_ctx), exception_handler_);

    return expected<const http_server&, std::error_code>(*this);
  }
#endif

  template <typename ResponseHandler>
    requires std::is_invocable_v<ResponseHandler, test_response>
      && co_awaitable<std::invoke_result_t<ResponseHandler, test_response>>
  void serve_request(std::string_view path,
                     request req,
                     ResponseHandler handler) const
  {
    auto target = [&]() -> std::string {
      boost::urls::url url;
      url.set_path(path);
      url.set_query(req.query().to_string());
      return std::string(url.encoded_target());
    }();

    net::co_spawn(ex_,
                  serve_request_impl(
                      std::move(req), std::move(target), std::move(handler)),
                  net::detached);
  }

private:
  template <typename ResponseHandler>
  auto serve_request_impl(request req,
                          std::string target,
                          ResponseHandler handler) const -> awaitable<void>
  {
    co_await handler(test_response(co_await do_handler(
        connection_info(
            net::ip::tcp::endpoint(net::ip::make_address("127.0.0.1"), 0),
            net::ip::tcp::endpoint(net::ip::make_address("127.0.0.1"), 0)),
        req.method(),
        http::version::v1_1,
        target,
        std::move(req.fields()),
        std::move(req.body()))));
  }

  auto do_listen(socket_acceptor acceptor) const -> awaitable<void>
  {
    for (;;) {
      if (auto socket = co_await acceptor.async_accept(use_awaitable); socket) {
        net::co_spawn(ex_,
                      do_session(shared_tcp_stream(std::move(*socket))),
                      exception_handler_);
      }
    }
  }

#if defined(FITORIA_HAS_OPENSSL)
  auto do_listen(socket_acceptor acceptor,
                 net::ssl::context& ssl_ctx) const -> awaitable<void>
  {
    for (;;) {
      if (auto socket = co_await acceptor.async_accept(use_awaitable); socket) {
        net::co_spawn(
            ex_,
            do_session(shared_ssl_stream(std::move(*socket), ssl_ctx)),
            exception_handler_);
      }
    }
  }
#endif

  auto do_session(shared_tcp_stream stream) const -> awaitable<void>
  {
    if (auto result = co_await do_session_impl(stream); !result) {
      FITORIA_THROW_OR(std::system_error(result.error()), co_return);
    }

    boost::system::error_code ec;
    stream.socket().shutdown(net::ip::tcp::socket::shutdown_send, ec); // NOLINT
  }

#if defined(FITORIA_HAS_OPENSSL)
  auto do_session(shared_ssl_stream stream) const -> awaitable<void>
  {
    if (auto result = co_await do_handshake(stream); !result) {
      FITORIA_THROW_OR(std::system_error(result.error()), co_return);
    }

    if (auto result = co_await do_session_impl(stream); !result) {
      FITORIA_THROW_OR(std::system_error(result.error()), co_return);
    }

    if (auto result = co_await stream.async_shutdown(use_awaitable); !result) {
      FITORIA_THROW_OR(std::system_error(result.error()), co_return);
    }
  }

  auto do_handshake(shared_ssl_stream& stream) const
      -> awaitable<expected<void, std::error_code>>
  {
    using namespace net::experimental::awaitable_operators;

    auto timer = net::steady_timer(stream.get_executor());
    if (tls_handshake_timeout_) {
      timer.expires_after(*tls_handshake_timeout_);
    } else {
      timer.expires_at(net::steady_timer::time_point::max());
    }

    auto result = co_await (
        stream.async_handshake(net::ssl::stream_base::server, use_awaitable)
        || timer.async_wait(use_awaitable));

    if (result.index() == 0) {
      co_return std::get<0>(result);
    }

    get_lowest_layer(stream).close();
    co_return unexpected { make_error_code(net::error::timed_out) };
  }
#endif

  template <typename Stream>
  auto do_session_impl(Stream& stream) const
      -> awaitable<expected<void, std::error_code>>
  {
    using boost::beast::http::buffer_body;
    using boost::beast::http::empty_body;
    using boost::beast::http::request_parser;
    using boost::beast::http::response;

    for (;;) {
      flat_buffer buffer;
      auto parser = std::make_shared<request_parser<buffer_body>>();
      parser->body_limit(boost::none);

      if (client_request_timeout_) {
        get_lowest_layer(stream).expires_after(*client_request_timeout_);
      }

      if (auto bytes_read
          = co_await async_read_header(stream, buffer, *parser, use_awaitable);
          !bytes_read) {
        co_return unexpected { bytes_read.error() };
      }

      if (auto it = parser->get().find(http::field::expect);
          it != parser->get().end()
          && iequals(it->value(),
                     http::fields::expect::one_hundred_continue())) {
        if (auto bytes_written = co_await async_write(
                stream,
                response<empty_body>(http::status::continue_, 11),
                use_awaitable);
            !bytes_written) {
          co_return unexpected { bytes_written.error() };
        }
      }

      bool keep_alive = parser->get().keep_alive();

      auto res = co_await do_handler(
          connection_info(get_lowest_layer(stream).socket().local_endpoint(),
                          get_lowest_layer(stream).socket().remote_endpoint()),
          parser->get().method(),
          http::to_version(parser->get().version()),
          std::string(parser->get().target()),
          http_fields::from_impl(parser->get()),
          [&]() -> any_async_readable_stream {
            if (parser->get().has_content_length() || parser->get().chunked()) {
              return async_message_parser_stream(
                  std::move(buffer), stream, parser);
            }

            return async_readable_vector_stream();
          }());

      if (auto exp = co_await std::visit(
              overloaded {
                  [&](any_body::null) {
                    return do_null_body_response(stream, res, keep_alive);
                  },
                  [&](any_body::sized) {
                    return do_sized_response(stream, res, keep_alive);
                  },
                  [&](any_body::chunked) {
                    return do_chunked_response(stream, res, keep_alive);
                  } },
              res.body().size());
          !exp) {
        co_return unexpected { exp.error() };
      }

      if (!keep_alive) {
        break;
      }
    }

    co_return expected<void, std::error_code>();
  }

  auto do_handler(connection_info connection_info,
                  http::verb method,
                  http::version version,
                  std::string target,
                  http_fields fields,
                  any_async_readable_stream body) const -> awaitable<response>
  {
    auto req_url = boost::urls::parse_origin_form(target);
    if (!req_url) {
      co_return response::bad_request()
          .set_field(http::field::content_type,
                     http::fields::content_type::plaintext())
          .set_body("request target is invalid");
    }

    if (auto route = router_.try_find(method, req_url.value().path()); route) {
      auto req
          = request(std::move(connection_info),
                    path_info(std::string(route->matcher().pattern()),
                              req_url->path(),
                              route->matcher().match(req_url->path()).value()),
                    method,
                    version,
                    query_map::from(req_url->params()),
                    std::move(fields),
                    std::move(body),
                    route->states());
      co_return co_await route->operator()(req);
    }

    co_return response::not_found()
        .set_field(http::field::content_type,
                   http::fields::content_type::plaintext())
        .set_body("request path is not found");
  }

  template <typename Stream>
  auto do_null_body_response(Stream& stream, response& res, bool keep_alive)
      const -> awaitable<expected<void, std::error_code>>
  {
    using boost::beast::http::empty_body;
    using boost::beast::http::response;

    auto r = response<empty_body>(res.status_code().value(), 11);
    res.fields().to_impl(r);
    r.keep_alive(keep_alive);
    r.prepare_payload();

    // boost.beast incorrectly handle some cases
    // https://datatracker.ietf.org/doc/html/rfc7230#section-3.3.2
    if (res.status_code().category() == http::status_class::informational
        || res.status_code().value() == http::status::no_content) {
      r.content_length(boost::none);
    }

    co_return co_await async_write(stream, r, use_awaitable);
  }

  template <typename Stream>
  auto do_sized_response(Stream& stream, response& res, bool keep_alive) const
      -> awaitable<expected<void, std::error_code>>
  {
    using boost::beast::http::response;
    using boost::beast::http::vector_body;

    auto r = response<vector_body<std::byte>>(res.status_code().value(), 11);
    res.fields().to_impl(r);
    if (auto data = co_await async_read_until_eof<std::vector<std::byte>>(
            res.body().stream());
        data) {
      r.body() = std::move(*data);
    } else if (data.error() != make_error_code(net::error::eof)) {
      co_return unexpected { data.error() };
    }
    r.keep_alive(keep_alive);
    r.prepare_payload();

    co_return co_await async_write(stream, r, use_awaitable);
  }

  template <typename Stream>
  auto do_chunked_response(Stream& stream, response& res, bool keep_alive) const
      -> awaitable<expected<void, std::error_code>>
  {
    using boost::beast::http::empty_body;
    using boost::beast::http::response;
    using boost::beast::http::response_serializer;

    auto r = response<empty_body>(res.status_code().value(), 11);
    res.fields().to_impl(r);
    r.keep_alive(keep_alive);
    r.chunked(true);

    auto ser = response_serializer<empty_body>(r);

    if (auto result = co_await async_write_header(stream, ser, use_awaitable);
        !result) {
      co_return unexpected { result.error() };
    }

    co_return co_await async_write_chunks(stream, res.body().stream());
  }

#if FITORIA_NO_EXCEPTIONS
  static constexpr auto default_exception_handler = exception_handler_t {};
#else
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
  optional<std::chrono::milliseconds> client_request_timeout_;
  optional<std::chrono::milliseconds> tls_handshake_timeout_;
  exception_handler_t exception_handler_;
};

class http_server_builder {
  using request_type = request&;
  using response_type = awaitable<response>;
  using router_type = router<request_type, response_type>;
#if FITORIA_NO_EXCEPTIONS
  using exception_handler_t = net::detached_t;
#else
  using exception_handler_t = std::function<void(std::exception_ptr)>;
#endif

public:
  http_server_builder(executor_type ex)
      : ex_(std::move(ex))
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

  auto set_client_request_timeout(
      optional<std::chrono::milliseconds> timeout) & noexcept
      -> http_server_builder&
  {
    client_request_timeout_ = timeout;
    return *this;
  }

  auto set_client_request_timeout(
      optional<std::chrono::milliseconds> timeout) && noexcept
      -> http_server_builder&&
  {
    set_client_request_timeout(timeout);
    return std::move(*this);
  }

  auto set_tls_handshake_timeout(optional<std::chrono::milliseconds>
                                     timeout) & noexcept -> http_server_builder&
  {
    tls_handshake_timeout_ = timeout;
    return *this;
  }

  auto set_tls_handshake_timeout(
      optional<std::chrono::milliseconds> timeout) && noexcept
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

    return { ex_,
             std::move(router_),
             max_listen_connections_,
             client_request_timeout_,
             tls_handshake_timeout_,
             std::move(exception_handler_) };
  }

private:
  executor_type ex_;
  router_type router_;
  optional<int> max_listen_connections_;
  optional<std::chrono::milliseconds> client_request_timeout_
      = std::chrono::seconds(5);
  optional<std::chrono::milliseconds> tls_handshake_timeout_
      = std::chrono::seconds(5);
  optional<exception_handler_t> exception_handler_;
};

}

FITORIA_NAMESPACE_END

#endif
