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
#include <fitoria/web/state_storage.hpp>
#include <fitoria/web/test_utils.hpp>
#include <fitoria/web/websocket.hpp>

#include <system_error>

FITORIA_NAMESPACE_BEGIN

namespace web {

/// @verbatim embed:rst:leading-slashes
///
/// An HTTP Server for handling client requests.
///
/// @endverbatim
class http_server {
  using request_type = request&;
  using response_type = awaitable<response>;
  using router_type = router<request_type, response_type>;
  using duration_type = std::chrono::steady_clock::duration;
#if FITORIA_NO_EXCEPTIONS
  using exception_handler_t = net::detached_t;
#else
  using exception_handler_t = std::function<void(std::exception_ptr)>;
#endif

  http_server(const executor_type& ex,
              router_type router,
              optional<int> max_listen_connections,
              optional<duration_type> tls_handshake_timeout,
              optional<duration_type> request_timeout,
              optional<std::uint32_t> request_header_limit,
              optional<std::uint64_t> request_body_limit,
              optional<exception_handler_t> exception_handler)
      : ex_(std::move(ex))
      , router_(std::move(router))
      , max_listen_connections_(max_listen_connections.value_or(
            static_cast<int>(net::socket_base::max_listen_connections)))
      , tls_handshake_timeout_(tls_handshake_timeout)
      , request_timeout_(request_timeout)
      , request_header_limit_(request_header_limit)
      , request_body_limit_(request_body_limit)
      , exception_handler_(
            exception_handler.value_or(default_exception_handler))
  {
  }

public:
  class builder;

  friend class builder;

  http_server(const http_server&) = delete;

  http_server& operator=(const http_server&) = delete;

  http_server(http_server&&) = delete;

  http_server& operator=(http_server&&) = delete;

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Get the number of maximum concurrent connections.
  ///
  /// @endverbatim
  auto max_listen_connections() const noexcept -> int
  {
    return max_listen_connections_;
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Get the timeout for TLS handshake.
  ///
  /// DESCRIPTION
  ///     Get the timeout for TLS handshake. If no timeout is set, ``nullopt``
  ///     is returned.
  ///
  /// @endverbatim
  auto tls_handshake_timeout() const noexcept -> optional<duration_type>
  {
    return tls_handshake_timeout_;
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Get the timeout for serving client request.
  ///
  /// DESCRIPTION
  ///     Get the timeout for serving client request. If no timeout is set,
  ///     ``nullopt`` is returned.
  ///
  /// @endverbatim
  auto request_timeout() const noexcept -> optional<duration_type>
  {
    return request_timeout_;
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Get the maximum size in bytes for reading client request headers.
  ///
  /// DESCRIPTION
  ///     Get the maximum size in bytes for reading client request headers.
  ///     ``nullopt`` indicates no size check will be performed.
  ///
  /// @endverbatim
  auto request_header_limit() const noexcept -> optional<std::uint32_t>
  {
    return request_header_limit_;
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Get the maximum size in bytes for reading client request body.
  ///
  /// DESCRIPTION
  ///     Get the maximum size in bytes for reading client request body.
  ///     ``nullopt`` indicates no size check will be performed.
  ///
  /// @endverbatim
  auto request_body_limit() const noexcept -> optional<std::uint64_t>
  {
    return request_body_limit_;
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Binds address and create an acceptor for TCP connections.
  ///
  /// @endverbatim
  auto bind(std::string_view ip_addr, std::uint16_t port) const
      -> expected<const http_server&, std::error_code>
  {
    auto endpoint = detail::make_endpoint(ip_addr, port);
    if (!endpoint) {
      return unexpected { endpoint.error() };
    }

    auto acceptor
        = detail::make_acceptor(ex_, *endpoint, max_listen_connections_);
    if (!acceptor) {
      return unexpected { acceptor.error() };
    }

    net::co_spawn(ex_, do_listen(std::move(*acceptor)), exception_handler_);

    return expected<const http_server&, std::error_code>(*this);
  }

#if defined(FITORIA_HAS_OPENSSL)

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Binds address and create an acceptor for TLS connections.
  ///
  /// @endverbatim
  auto bind(std::string_view ip_addr,
            std::uint16_t port,
            net::ssl::context& ssl_ctx) const
      -> expected<const http_server&, std::error_code>
  {
    auto endpoint = detail::make_endpoint(ip_addr, port);
    if (!endpoint) {
      return unexpected { endpoint.error() };
    }

    auto acceptor
        = detail::make_acceptor(ex_, *endpoint, max_listen_connections_);
    if (!acceptor) {
      return unexpected { acceptor.error() };
    }

    net::co_spawn(
        ex_, do_listen(std::move(*acceptor), ssl_ctx), exception_handler_);

    return expected<const http_server&, std::error_code>(*this);
  }

#endif

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Binds address and create an acceptor for local (Unix domain socket)
  /// connections.
  ///
  /// DESCRIPTION
  ///     Binds address and create an acceptor for local (Unix domain socket)
  ///     connections. Make sure that no file is at the ``file_path``, or system
  ///     call ``bind`` may return error.
  ///
  /// @endverbatim
  auto bind_local(std::string_view file_path) const
      -> expected<const http_server&, std::error_code>
  {
    auto endpoint = detail::make_endpoint(file_path);
    if (!endpoint) {
      return unexpected { endpoint.error() };
    }

    auto acceptor
        = detail::make_acceptor(ex_, *endpoint, max_listen_connections_);
    if (!acceptor) {
      return unexpected { acceptor.error() };
    }

    net::co_spawn(ex_, do_listen(std::move(*acceptor)), exception_handler_);

    return expected<const http_server&, std::error_code>(*this);
  }

#if defined(FITORIA_HAS_OPENSSL)

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Binds address and create an acceptor for local (Unix domain socket) TLS
  /// connections.
  ///
  /// DESCRIPTION
  ///     Binds address and create an acceptor for local (Unix domain socket)
  ///     TLS connections. Make sure that no file is at the ``file_path``, or
  ///     system call ``bind`` may return error.
  ///
  /// @endverbatim
  auto bind_local(std::string_view file_path, net::ssl::context& ssl_ctx) const
      -> expected<const http_server&, std::error_code>
  {
    auto endpoint = detail::make_endpoint(file_path);
    if (!endpoint) {
      return unexpected { endpoint.error() };
    }

    auto acceptor
        = detail::make_acceptor(ex_, *endpoint, max_listen_connections_);
    if (!acceptor) {
      return unexpected { acceptor.error() };
    }

    net::co_spawn(
        ex_, do_listen(std::move(*acceptor), ssl_ctx), exception_handler_);

    return expected<const http_server&, std::error_code>(*this);
  }

#endif

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Provides testing with a mock ``test_request`` against the ``http_server``
  /// using fake connections.
  ///
  /// @endverbatim
  template <typename ResponseHandler>
    requires std::is_invocable_v<ResponseHandler, test_response>
      && co_awaitable<std::invoke_result_t<ResponseHandler, test_response>>
  void serve_request(std::string path,
                     test_request req,
                     ResponseHandler handler) const
  {
    auto stream = shared_test_stream(ex_);
    net::co_spawn(ex_, do_session(connect(stream)), net::detached);
    net::co_spawn(
        ex_,
        [](shared_test_stream stream,
           std::string path,
           test_request tr,
           ResponseHandler handler) -> awaitable<void> {
          co_await handler(*(co_await do_http_request(stream, path, tr)));
        }(stream, std::move(path), std::move(req), std::move(handler)),
        net::detached);
  }

private:
  template <typename Protocol>
  auto do_listen(socket_acceptor<Protocol> acceptor) const -> awaitable<void>
  {
    for (;;) {
      if (auto socket = co_await acceptor.async_accept(use_awaitable); socket) {
        net::co_spawn(
            ex_,
            do_session(shared_basic_stream<Protocol>(std::move(*socket))),
            exception_handler_);
      }
    }
  }

#if defined(FITORIA_HAS_OPENSSL)
  template <typename Protocol>
  auto do_listen(socket_acceptor<Protocol> acceptor,
                 net::ssl::context& ssl_ctx) const -> awaitable<void>
  {
    for (;;) {
      if (auto socket = co_await acceptor.async_accept(use_awaitable); socket) {
        net::co_spawn(ex_,
                      do_session(shared_ssl_stream<Protocol>(std::move(*socket),
                                                             ssl_ctx)),
                      exception_handler_);
      }
    }
  }
#endif

  template <typename Stream>
  auto do_session(Stream stream) const -> awaitable<void>
  {
    if (auto result = co_await do_session_impl(stream); !result) {
      FITORIA_THROW_OR(std::system_error(result.error()), co_return);
    }

    boost::system::error_code ec;
    stream.shutdown(net::ip::tcp::socket::shutdown_send, ec);
  }

#if defined(FITORIA_HAS_OPENSSL)
  template <typename Protocol>
  auto do_session(shared_ssl_stream<Protocol> stream) const -> awaitable<void>
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

  template <typename Protocol>
  auto do_handshake(shared_ssl_stream<Protocol>& stream) const
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
    using boost::beast::websocket::is_upgrade;

    auto buffer = std::make_shared<flat_buffer>();

    for (;;) {
      auto parser = std::make_shared<request_parser<buffer_body>>();
      parser->header_limit(request_header_limit_.value_or(UINT32_MAX));
      parser->body_limit(request_body_limit_.value_or(UINT64_MAX));

      if (request_timeout_) {
        get_lowest_layer(stream).expires_after(*request_timeout_);
      }

      if (auto bytes_read
          = co_await async_read_header(stream, *buffer, *parser, use_awaitable);
          !bytes_read) {
        if (bytes_read.error()
            == make_error_code(boost::beast::http::error::header_limit)) {
          auto res
              = web::response::bad_request()
                    .set_header(http::field::content_type, mime::text_plain())
                    .set_body("request headers size exceeds limit");
          co_return co_await do_response(stream, res, false);
        } else {
          co_return unexpected { bytes_read.error() };
        }
      }

      const bool keep_alive = parser->get().keep_alive();
      const bool upgrade = is_upgrade(parser->get());
      auto session_state = std::make_shared<state_map>();

      if (upgrade) {
        // timeout must be turned off, websocket has its own timeout mechanism
        get_lowest_layer(stream).expires_never();

        // we don't handle expect: 100-continue here,
        // boost::beast::websocket::stream::accept() will do it for us
        (*session_state)[std::type_index(typeid(websocket))]
            = websocket(stream);
      } else {
        if (auto it = parser->get().find(http::field::expect);
            it != parser->get().end() && iequals(it->value(), "100-continue")) {
          if (auto bytes_written = co_await async_write(
                  stream,
                  response<empty_body>(http::status::continue_, 11),
                  use_awaitable);
              !bytes_written) {
            co_return unexpected { bytes_written.error() };
          }
        }
      }

      auto res = web::response();
      if (auto url = boost::urls::parse_origin_form(parser->get().target());
          url) {
        if (auto route = router_.try_find(parser->get().method(), url->path());
            route) {
          auto req = web::request(
              connect_info(get_lowest_layer(stream).socket()),
              path_info(std::string(route->matcher().pattern()),
                        url->path(),
                        route->matcher().match(url->path()).value()),
              parser->get().method(),
              http::detail::from_impl_version(parser->get().version()),
              http::header_map::from_impl(parser->get()),
              query_map::from(url->params()),
              [&]() -> any_async_readable_stream {
                if (parser->get().has_content_length()
                    || parser->get().chunked()) {
                  return async_message_parser_stream(buffer, stream, parser);
                }

                return async_readable_vector_stream();
              }(),
              route->states().copy_prepend(session_state));
          res = co_await route->operator()(req);
        } else {
          res = web::response::not_found()
                    .set_header(http::field::content_type, mime::text_plain())
                    .set_body("request path is not found");
        }
      } else {
        res = web::response::bad_request()
                  .set_header(http::field::content_type, mime::text_plain())
                  .set_body("request target is invalid");
      }

      if (upgrade) {
        auto& ws = *std::any_cast<websocket>(
            &(*session_state)[std::type_index(typeid(websocket))]);
        ws.set_response(std::move(res));
        if (auto result = co_await ws.run(parser->get()); !result) {
          co_return unexpected { result.error() };
        }
      } else {
        if (auto exp = co_await do_response(stream, res, keep_alive); !exp) {
          co_return unexpected { exp.error() };
        }
      }

      if (!keep_alive) {
        break;
      }
    }

    co_return expected<void, std::error_code>();
  }

  template <typename Stream>
  auto do_response(Stream& stream, response& res, bool keep_alive) const
      -> awaitable<expected<void, std::error_code>>
  {
    co_return co_await std::visit(
        overloaded { [&](any_body::null) {
                      return do_null_body_response(stream, res, keep_alive);
                    },
                     [&](any_body::sized) {
                       return do_sized_response(stream, res, keep_alive);
                     },
                     [&](any_body::chunked) {
                       return do_chunked_response(stream, res, keep_alive);
                     } },
        res.body().size());
  }

  template <typename Stream>
  auto do_null_body_response(Stream& stream, response& res, bool keep_alive)
      const -> awaitable<expected<void, std::error_code>>
  {
    using boost::beast::http::empty_body;
    using boost::beast::http::response;

    auto r = response<empty_body>(res.status_code().value(),
                                  http::detail::to_impl_version(res.version()));
    res.headers().to_impl(r);
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

    auto r = response<vector_body<std::byte>>(
        res.status_code().value(),
        http::detail::to_impl_version(res.version()));
    res.headers().to_impl(r);
    if (auto data = co_await async_read_until_eof<bytes>(res.body().stream());
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

    auto r = response<empty_body>(res.status_code().value(),
                                  http::detail::to_impl_version(res.version()));
    res.headers().to_impl(r);
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
  optional<duration_type> tls_handshake_timeout_;
  optional<duration_type> request_timeout_;
  optional<std::uint32_t> request_header_limit_;
  optional<std::uint64_t> request_body_limit_;
  exception_handler_t exception_handler_;
};

class http_server::builder {
public:
  explicit builder(const executor_type& ex)
      : ex_(std::move(ex))
  {
  }

  template <typename ExecutionContext>
    requires std::is_convertible_v<ExecutionContext&, net::execution_context&>
  builder(ExecutionContext& context)
      : ex_(context.get_executor())
  {
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Set the number of maximum concurrent connections.
  ///
  /// @endverbatim
  auto set_max_listen_connections(int num) & noexcept -> builder&
  {
    max_listen_connections_ = num;
    return *this;
  }

  auto set_max_listen_connections(int num) && noexcept -> builder&&
  {
    set_max_listen_connections(num);
    return std::move(*this);
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Set the timeout for TLS handshake.
  ///
  /// DESCRIPTION
  ///     Pass ``nullopt`` to disable timeout.
  ///
  /// @endverbatim
  auto set_tls_handshake_timeout(optional<duration_type> timeout) & noexcept
      -> builder&
  {
    tls_handshake_timeout_ = timeout;
    return *this;
  }

  auto set_tls_handshake_timeout(optional<duration_type> timeout) && noexcept
      -> builder&&
  {
    set_tls_handshake_timeout(timeout);
    return std::move(*this);
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Set the timeout for serving client request.
  ///
  /// DESCRIPTION
  ///     Pass ``nullopt`` to disable timeout.
  ///
  /// @endverbatim
  auto
  set_request_timeout(optional<duration_type> timeout) & noexcept -> builder&
  {
    request_timeout_ = timeout;
    return *this;
  }

  auto
  set_request_timeout(optional<duration_type> timeout) && noexcept -> builder&&
  {
    set_request_timeout(timeout);
    return std::move(*this);
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Set the maximum size in bytes for reading client request headers.
  ///
  /// DESCRIPTION
  ///     Set the maximum size in bytes for reading client request headers. Pass
  ///     ``nullopt`` to disable headers size check. Default limit is 8KB.
  ///
  /// @endverbatim
  auto
  set_request_header_limit(optional<std::uint32_t> limit) & noexcept -> builder&
  {
    request_header_limit_ = limit;
    return *this;
  }

  auto set_request_header_limit(optional<std::uint32_t> limit) && noexcept
      -> builder&&
  {
    set_request_header_limit(limit);
    return std::move(*this);
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Set the maximum size in bytes for reading client request body.
  ///
  /// DESCRIPTION
  ///     Set the maximum size in bytes for reading client request body. Pass
  ///     ``nullopt`` to disable body size check. Default limit is 1MB.
  ///
  /// @endverbatim
  auto
  set_request_body_limit(optional<std::uint64_t> limit) & noexcept -> builder&
  {
    request_body_limit_ = limit;
    return *this;
  }

  auto
  set_request_body_limit(optional<std::uint64_t> limit) && noexcept -> builder&&
  {
    set_request_body_limit(limit);
    return std::move(*this);
  }

#if !FITORIA_NO_EXCEPTIONS

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Set the exception handler to handle any exception thrown from the server.
  ///
  /// DESCRIPTION
  ///     Set the exception handler to handle any exception thrown from the
  ///     server. If is not set by user, the ``default_exception_handler`` will
  ///     be used.
  ///
  /// @endverbatim
  template <typename F>
    requires std::invocable<F, std::exception_ptr>
  auto set_exception_handler(F&& f) & -> builder&
  {
    exception_handler_.emplace(std::forward<F>(f));
    return *this;
  }

  template <typename F>
    requires std::invocable<F, std::exception_ptr>
  auto set_exception_handler(F&& f) && -> builder&&
  {
    set_exception_handler(std::forward<F>(f));
    return std::move(*this);
  }
#endif

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Add a ``route`` to the server.
  ///
  /// DESCRIPTION
  ///     Add a ``route`` to the server. Note that no duplicate ``route`` s are
  ///     allowed.
  ///
  /// @endverbatim
  template <basic_fixed_string RoutePath,
            typename... RouteServices,
            typename Handler>
  auto serve(route_impl<RoutePath, std::tuple<RouteServices...>, Handler>
                 route) & -> builder&
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
                 route) && -> builder&&
  {
    serve(std::move(route));
    return std::move(*this);
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Add all ``route`` s from a ``scope`` to the server.
  ///
  /// DESCRIPTION
  ///     Add all ``route`` s from a ``scope`` to the server. Note that no
  ///     duplicate routes are allowed.
  ///
  /// @endverbatim
  template <basic_fixed_string Path, typename... Services, typename... Routes>
  auto serve(scope_impl<Path, std::tuple<Services...>, std::tuple<Routes...>>
                 scope) & -> builder&
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
                 scope) && -> builder&&
  {
    serve(std::move(scope));
    return std::move(*this);
  }

  /// @verbatim embed:rst:leading-slashes
  ///
  /// Create the ``http_server``.
  ///
  /// DESCRIPTION
  ///     Create the ``http_server``. Note that current object is no longer
  ///     usable after calling this function.
  ///
  /// @endverbatim
  auto build() -> http_server
  {
    router_.optimize();

    return { ex_,
             std::move(router_),
             max_listen_connections_,
             tls_handshake_timeout_,
             request_timeout_,
             request_header_limit_,
             request_body_limit_,
             std::move(exception_handler_) };
  }

private:
  executor_type ex_;
  router_type router_;
  optional<int> max_listen_connections_;
  optional<duration_type> tls_handshake_timeout_ = std::chrono::seconds(3);
  optional<duration_type> request_timeout_ = std::chrono::seconds(5);
  optional<std::uint32_t> request_header_limit_ = 8 * 1024;
  optional<std::uint64_t> request_body_limit_ = 1 * 1024 * 1024;
  optional<exception_handler_t> exception_handler_;
};

}

FITORIA_NAMESPACE_END

#endif
