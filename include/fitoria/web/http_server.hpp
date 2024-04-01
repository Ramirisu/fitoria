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

#include <fitoria/core/net.hpp>
#include <fitoria/core/url.hpp>

#include <fitoria/log.hpp>

#include <fitoria/web/async_message_parser_stream.hpp>
#include <fitoria/web/handler.hpp>
#include <fitoria/web/http_context.hpp>
#include <fitoria/web/http_response.hpp>
#include <fitoria/web/router.hpp>
#include <fitoria/web/scope.hpp>

#include <system_error>

FITORIA_NAMESPACE_BEGIN

namespace web {

class http_server {
  using router_type = router<http_context&, net::awaitable<http_response>>;

public:
  class builder {
    friend class http_server;

  public:
    http_server build()
    {
      return http_server(std::move(*this));
    }

    template <typename F>
    builder& configure(F&& f)
      requires std::invocable<F, builder&>
    {
      std::invoke(std::forward<F>(f), *this);
      return *this;
    }

    builder& set_max_listen_connections(int num) noexcept
    {
      max_listen_connections_ = num;
      return *this;
    }

    builder&
    set_client_request_timeout(std::chrono::milliseconds timeout) noexcept
    {
      client_request_timeout_ = timeout;
      return *this;
    }

#if !FITORIA_NO_EXCEPTIONS
    template <typename F>
    builder& set_exception_handler(F&& f)
      requires std::invocable<F, std::exception_ptr>
    {
      exception_handler_.emplace(std::forward<F>(f));
      return *this;
    }
#endif

    template <basic_fixed_string RoutePath,
              typename... RouteServices,
              typename Handler>
    builder&
    serve(route_impl<RoutePath, std::tuple<RouteServices...>, Handler> route)
    {
      if (auto res
          = router_.try_insert(router_type::route_type(route.build(handler())));
          !res) {
        FITORIA_THROW_OR(std::system_error(res.error()), std::terminate());
      }

      return *this;
    }

    template <basic_fixed_string Path, typename... Services, typename... Routes>
    builder& serve(
        scope_impl<Path, std::tuple<Services...>, std::tuple<Routes...>> scope)
    {
      std::apply(
          [this](auto&&... routes) {
            (this->serve(std::forward<decltype(routes)>(routes)), ...);
          },
          scope.routes());

      return *this;
    }

  private:
    static const char* name() noexcept
    {
      return "fitoria.builder";
    }

    optional<int> max_listen_connections_;
    optional<std::chrono::milliseconds> client_request_timeout_;
#if !FITORIA_NO_EXCEPTIONS
    optional<std::function<void(std::exception_ptr)>> exception_handler_;
#else
    net::detached_t exception_handler_;
#endif
    router_type router_;
  };

  http_server(builder builder)
      : max_listen_connections_(builder.max_listen_connections_.value_or(
          static_cast<int>(net::socket_base::max_listen_connections)))
      , client_request_timeout_(
            builder.client_request_timeout_.value_or(std::chrono::seconds(5)))
#if !FITORIA_NO_EXCEPTIONS
      , exception_handler_(
            builder.exception_handler_.value_or(default_exception_handler))
#endif
      , router_(std::move(builder.router_))
  {
  }

  int max_listen_connections() const noexcept
  {
    return max_listen_connections_;
  }

  std::chrono::milliseconds client_request_timeout() const noexcept
  {
    return client_request_timeout_;
  }

  http_server& bind(std::string_view addr, std::uint16_t port)
  {
    tasks_.push_back(
        do_listen(net::ip::tcp::endpoint(net::ip::make_address(addr), port)));

    return *this;
  }

#if defined(FITORIA_HAS_OPENSSL)
  http_server&
  bind_ssl(std::string_view addr, std::uint16_t port, net::ssl::context ssl_ctx)
  {
    tasks_.push_back(
        do_listen(net::ip::tcp::endpoint(net::ip::make_address(addr), port),
                  std::move(ssl_ctx)));

    return *this;
  }
#endif

  void run(std::uint32_t threads = std::thread::hardware_concurrency())
  {
    log::info("[{}] starting server with {} workers", name(), threads);
    net::io_context ioc(threads);
    run_impl(ioc);
    net::thread_pool tp(threads);
    for (auto i = std::uint32_t(1); i < threads; ++i) {
      net::post(tp, [&]() { ioc.run(); });
    }
    ioc.run();
  }

  net::awaitable<void> async_run()
  {
    log::info("[{}] starting server", name());
    run_impl(co_await net::this_coro::executor);
  }

  net::awaitable<http_response> async_serve_request(std::string_view path,
                                                    http_request req)
  {
    boost::urls::url url;
    url.set_path(path);
    url.set_query(req.query().to_string());

    return do_handler(
        connection_info {
            net::ip::tcp::endpoint(net::ip::make_address("127.0.0.1"), 0),
            net::ip::tcp::endpoint(net::ip::make_address("127.0.0.1"), 0),
            net::ip::tcp::endpoint(net::ip::make_address("127.0.0.1"), 0) },
        req.method(),
        std::string(url.encoded_target()),
        std::move(req.fields()),
        std::move(req.body()));
  }

private:
  template <typename Executor>
  void run_impl(Executor&& executor)
  {
    for (auto& task : tasks_) {
      net::co_spawn(executor, std::move(task), exception_handler_);
    }
    tasks_.clear();
  }

  net::awaitable<net::ip::tcp::acceptor>
  new_acceptor(net::ip::tcp::endpoint endpoint) const
  {
    auto acceptor = net::ip::tcp::acceptor(co_await net::this_coro::executor);

    acceptor.open(endpoint.protocol());
    acceptor.set_option(net::socket_base::reuse_address(true));
    acceptor.bind(endpoint);
    acceptor.listen(max_listen_connections_);
    co_return acceptor;
  }

  net::awaitable<void> do_listen(net::ip::tcp::endpoint endpoint) const
  {
    auto acceptor = co_await new_acceptor(endpoint);

    for (;;) {
      auto [ec, socket] = co_await acceptor.async_accept(net::use_ta);
      if (ec) {
        continue;
      }

      net::co_spawn(
          acceptor.get_executor(),
          do_session(net::shared_tcp_stream(std::move(socket)), endpoint),
          exception_handler_);
    }
  }

#if defined(FITORIA_HAS_OPENSSL)
  net::awaitable<void> do_listen(net::ip::tcp::endpoint endpoint,
                                 net::ssl::context ssl_ctx) const
  {
    auto acceptor = co_await new_acceptor(endpoint);
    auto ssl_ctx_ptr = std::make_shared<net::ssl::context>(std::move(ssl_ctx));

    for (;;) {
      auto [ec, socket] = co_await acceptor.async_accept(net::use_ta);
      if (ec) {
        continue;
      }

      net::co_spawn(
          acceptor.get_executor(),
          do_session(net::shared_ssl_stream(std::move(socket), ssl_ctx_ptr),
                     endpoint),
          exception_handler_);
    }
  }
#endif

  net::awaitable<void> do_session(net::shared_tcp_stream stream,
                                  net::ip::tcp::endpoint listen_ep) const
  {
    if (auto ec = co_await do_session_impl(stream, std::move(listen_ep)); ec) {
      FITORIA_THROW_OR(std::system_error(ec), co_return);
    }

    boost::system::error_code ec;
    stream->socket().shutdown(net::ip::tcp::socket::shutdown_send, ec);
  }

#if defined(FITORIA_HAS_OPENSSL)
  net::awaitable<void> do_session(net::shared_ssl_stream stream,
                                  net::ip::tcp::endpoint listen_ep) const
  {
    boost::system::error_code ec;

    boost::beast::get_lowest_layer(*stream).expires_after(
        client_request_timeout_);
    std::tie(ec) = co_await stream->async_handshake(
        net::ssl::stream_base::server, net::use_ta);
    if (ec) {
      FITORIA_THROW_OR(std::system_error(ec), co_return);
    }

    if (ec = co_await do_session_impl(stream, std::move(listen_ep)); ec) {
      FITORIA_THROW_OR(std::system_error(ec), co_return);
    }

    std::tie(ec) = co_await stream->async_shutdown(net::use_ta);
    if (ec) {
      FITORIA_THROW_OR(std::system_error(ec), co_return);
    }
  }
#endif

  template <typename Stream>
  net::awaitable<std::error_code>
  do_session_impl(Stream& stream, net::ip::tcp::endpoint listen_ep) const
  {
    using boost::beast::http::buffer_body;
    using boost::beast::http::empty_body;
    using boost::beast::http::request_parser;
    using boost::beast::http::response;

    boost::system::error_code ec;

    for (;;) {
      boost::beast::flat_buffer buffer;
      auto parser = std::make_unique<request_parser<buffer_body>>();
      parser->body_limit(boost::none);

      boost::beast::get_lowest_layer(*stream).expires_after(
          client_request_timeout_);
      std::tie(ec, std::ignore)
          = co_await async_read_header(*stream, buffer, *parser, net::use_ta);
      if (ec) {
        co_return ec;
      }

      bool keep_alive = parser->get().keep_alive();
      auto method = parser->get().method();
      auto target = std::string(parser->get().target());
      auto fields = http_fields::from_impl(parser->get());

      if (auto it = parser->get().find(http::field::expect);
          it != parser->get().end() && it->value() == "100-continue") {
        boost::beast::get_lowest_layer(*stream).expires_after(
            client_request_timeout_);
        std::tie(ec, std::ignore) = co_await async_write(
            *stream,
            response<empty_body>(http::status::continue_, 11),
            net::use_ta);
        if (ec) {
          co_return ec;
        }
      }

      auto res = co_await do_handler(
          connection_info { net::get_local_endpoint(*stream),
                            net::get_remote_endpoint(*stream),
                            std::move(listen_ep) },
          method,
          std::move(target),
          std::move(fields),
          async_message_parser_stream(std::move(buffer),
                                      stream,
                                      std::move(parser),
                                      client_request_timeout_));

      auto do_response
          = [&]() -> net::awaitable<expected<void, std::error_code>> {
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

  net::awaitable<http_response> do_handler(connection_info connection_info,
                                           http::verb method,
                                           std::string target,
                                           http_fields fields,
                                           any_async_readable_stream body) const
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
      auto context = http_context(request);
      co_return co_await route->operator()(context);
    }

    co_return http_response(http::status::not_found)
        .set_field(http::field::content_type,
                   http::fields::content_type::plaintext())
        .set_body("request path is not found");
  }

  template <typename Stream>
  auto
  do_sized_response(Stream& stream, http_response& res, bool keep_alive) const
      -> net::awaitable<expected<void, std::error_code>>
  {
    using boost::beast::http::response;
    using boost::beast::http::vector_body;

    boost::system::error_code ec;

    auto r = response<vector_body<std::byte>>(res.status_code().value(), 11);
    res.fields().to_impl(r);
    if (auto body
        = co_await async_read_all_as<std::vector<std::byte>>(res.body());
        body) {
      if (!*body) {
        co_return *body;
      }
      r.body() = std::move(**body);
    }
    r.keep_alive(keep_alive);
    r.prepare_payload();

    boost::beast::get_lowest_layer(*stream).expires_after(
        client_request_timeout_);
    std::tie(ec, std::ignore) = co_await async_write(*stream, r, net::use_ta);
    if (ec) {
      co_return unexpected { ec };
    }

    co_return expected<void, std::error_code>();
  }

  template <typename Stream>
  auto
  do_chunked_response(Stream& stream, http_response& res, bool keep_alive) const
      -> net::awaitable<expected<void, std::error_code>>
  {
    using boost::beast::http::empty_body;
    using boost::beast::http::response;
    using boost::beast::http::response_serializer;

    boost::system::error_code ec;

    auto r = response<empty_body>(res.status_code().value(), 11);
    res.fields().to_impl(r);
    r.keep_alive(keep_alive);
    r.chunked(true);

    auto serializer = response_serializer<empty_body>(std::move(r));

    boost::beast::get_lowest_layer(*stream).expires_after(
        client_request_timeout_);
    std::tie(ec, std::ignore)
        = co_await async_write_header(*stream, serializer, net::use_ta);
    if (ec) {
      co_return unexpected { ec };
    }

    co_return co_await async_write_each_chunk(
        *stream, res.body(), client_request_timeout_);
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

  // from builder
  int max_listen_connections_;
  std::chrono::milliseconds client_request_timeout_;
#if !FITORIA_NO_EXCEPTIONS
  std::function<void(std::exception_ptr)> exception_handler_;
#else
  net::detached_t exception_handler_;
#endif
  router_type router_;

  // others
  std::vector<net::awaitable<void>> tasks_;
};
}

FITORIA_NAMESPACE_END

#endif
