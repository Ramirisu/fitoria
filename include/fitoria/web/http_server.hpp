//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef FITORIA_WEB_HTTP_SERVER_HPP
#define FITORIA_WEB_HTTP_SERVER_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/core/lazy.hpp>
#include <fitoria/core/net.hpp>
#include <fitoria/core/url.hpp>

#include <fitoria/log.hpp>

#include <fitoria/web/async_message_parser_stream.hpp>
#include <fitoria/web/handler.hpp>
#include <fitoria/web/http_context.hpp>
#include <fitoria/web/http_response.hpp>
#include <fitoria/web/router.hpp>
#include <fitoria/web/scope.hpp>

FITORIA_NAMESPACE_BEGIN

namespace web {

class http_server {
  using router_type = router<http_context&, lazy<http_response>>;

public:
  class builder {
    friend class http_server;

  public:
    http_server build()
    {
      return http_server(*this);
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

    template <typename F>
    builder& set_network_error_handler(F&& f)
      requires std::invocable<F, net::error_code>
    {
      network_error_handler_ = std::forward<F>(f);
      return *this;
    }

#if !FITORIA_NO_EXCEPTIONS
    template <typename F>
    builder& set_exception_handler(F&& f)
      requires std::invocable<F, std::exception_ptr>
    {
      exception_handler_ = std::forward<F>(f);
      return *this;
    }
#endif

    template <basic_fixed_string RoutePath,
              typename... RouteServices,
              typename Handler>
    builder&
    route(route_builder<RoutePath, std::tuple<RouteServices...>, Handler> route)
    {
      if (auto res
          = router_.try_insert(router_type::route_type(route.build(handler())));
          !res) {
#if !FITORIA_NO_EXCEPTIONS
        throw system_error(res.error());
#else
        std::terminate();
#endif
      }

      return *this;
    }

    template <basic_fixed_string Path, typename... Services, typename... Routes>
    builder& route(
        scope_impl<Path, std::tuple<Services...>, std::tuple<Routes...>> scope)
    {
      std::apply(
          [this](auto&&... routes) {
            (this->route(std::forward<decltype(routes)>(routes)), ...);
          },
          scope.routes());

      return *this;
    }

  private:
    void handle_network_error(net::error_code ec) const
    {
      if (network_error_handler_) {
        network_error_handler_(ec);
      }
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
      return "fitoria.builder";
    }

    int max_listen_connections_ = net::socket_base::max_listen_connections;
    std::chrono::milliseconds client_request_timeout_ = std::chrono::seconds(5);
    std::function<void(net::error_code)> network_error_handler_;
#if !FITORIA_NO_EXCEPTIONS
    std::function<void(std::exception_ptr)> exception_handler_
        = default_exception_handler;
#else
    net::detached_t exception_handler_;
#endif
    router_type router_;
  };

  http_server(builder builder)
      : builder_(std::move(builder))
  {
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

  lazy<void> async_run()
  {
    log::info("[{}] starting server", name());
    run_impl(co_await net::this_coro::executor);
    co_return;
  }

  lazy<http_response> async_serve_request(std::string_view path,
                                          http_request req)
  {
    boost::urls::url url;
    url.set_path(path);
    url.set_query(req.query().to_string());

    return do_handler(connection_info { net::ip::make_address("127.0.0.1"),
                                        0,
                                        net::ip::make_address("127.0.0.1"),
                                        0,
                                        net::ip::make_address("127.0.0.1"),
                                        0 },
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
      net::co_spawn(executor, std::move(task), builder_.exception_handler_);
    }
    tasks_.clear();
  }

  lazy<net::acceptor> new_acceptor(net::ip::tcp::endpoint endpoint) const
  {
    auto acceptor = net::acceptor(co_await net::this_coro::executor);

    acceptor.open(endpoint.protocol());
    acceptor.set_option(net::socket_base::reuse_address(true));
    acceptor.bind(endpoint);
    acceptor.listen(builder_.max_listen_connections_);
    co_return acceptor;
  }

  lazy<void> do_listen(net::ip::tcp::endpoint endpoint) const
  {
    auto acceptor = co_await new_acceptor(endpoint);

    for (;;) {
      auto [ec, socket] = co_await acceptor.async_accept();
      if (ec) {
        log::debug("[{}] async_accept failed: {}", name(), ec.message());
        builder_.handle_network_error(ec);
        continue;
      }

      net::co_spawn(
          acceptor.get_executor(),
          do_session(std::make_shared<net::tcp_stream>(std::move(socket)),
                     endpoint),
          builder_.exception_handler_);
    }
  }

#if defined(FITORIA_HAS_OPENSSL)
  lazy<void> do_listen(net::ip::tcp::endpoint endpoint,
                       net::ssl::context ssl_ctx) const
  {
    auto acceptor = co_await new_acceptor(endpoint);

    for (;;) {
      auto [ec, socket] = co_await acceptor.async_accept();
      if (ec) {
        log::debug("[{}] async_accept failed: {}", name(), ec.message());
        builder_.handle_network_error(ec);
        continue;
      }

      net::co_spawn(acceptor.get_executor(),
                    do_session(std::make_shared<net::ssl_stream>(
                                   std::move(socket), ssl_ctx),
                               endpoint),
                    builder_.exception_handler_);
    }
  }
#endif

  lazy<void> do_session(std::shared_ptr<net::tcp_stream> stream,
                        net::ip::tcp::endpoint listen_ep) const
  {
    auto ec = co_await do_session_impl(stream, std::move(listen_ep));
    if (ec) {
      builder_.handle_network_error(ec);
      co_return;
    }

    stream->socket().shutdown(net::ip::tcp::socket::shutdown_send, ec);
  }

#if defined(FITORIA_HAS_OPENSSL)
  lazy<void> do_session(std::shared_ptr<net::ssl_stream> stream,
                        net::ip::tcp::endpoint listen_ep) const
  {
    using std::tie;

    net::error_code ec;

    net::get_lowest_layer(*stream).expires_after(
        builder_.client_request_timeout_);
    tie(ec) = co_await stream->async_handshake(net::ssl::stream_base::server);
    if (ec) {
      log::debug("[{}] async_handshake failed: {}", name(), ec.message());
      builder_.handle_network_error(ec);
      co_return;
    }

    if (ec = co_await do_session_impl(stream, std::move(listen_ep)); ec) {
      builder_.handle_network_error(ec);
      co_return;
    }

    tie(ec) = co_await stream->async_shutdown();
    if (ec) {
      log::debug("[{}] async_shutdown failed: {}", name(), ec.message());
      builder_.handle_network_error(ec);
      co_return;
    }
  }
#endif

  template <typename Stream>
  lazy<net::error_code> do_session_impl(std::shared_ptr<Stream> stream,
                                        net::ip::tcp::endpoint listen_ep) const
  {
    using boost::beast::http::empty_body;
    using boost::beast::http::request_parser;
    using boost::beast::http::response;
    using boost::beast::http::vector_body;
    using std::tie;
    auto _ = std::ignore;

    net::error_code ec;

    for (;;) {
      net::flat_buffer buffer;
      auto req_parser
          = std::make_unique<request_parser<vector_body<std::byte>>>();
      req_parser->body_limit(boost::none);

      net::get_lowest_layer(*stream).expires_after(
          builder_.client_request_timeout_);
      tie(ec, _) = co_await async_read_header(*stream, buffer, *req_parser);
      if (ec) {
        if (ec != http::error::end_of_stream) {
          log::debug("[{}] async_read_header failed: {}", name(), ec.message());
        }
        co_return ec;
      }

      bool keep_alive = req_parser->get().keep_alive();
      auto method = req_parser->get().method();
      auto target = std::string(req_parser->get().target());
      auto fields = http_fields::from(req_parser->get());
      bool chunked = req_parser->get().chunked();

      if (auto it = req_parser->get().find(http::field::expect);
          it != req_parser->get().end() && it->value() == "100-continue") {
        net::get_lowest_layer(*stream).expires_after(
            builder_.client_request_timeout_);
        tie(ec, _) = co_await async_write(
            *stream, response<empty_body>(http::status::continue_, 11));
        if (ec) {
          log::debug(
              "[{}] async_write 100-continue failed: {}", name(), ec.message());
          co_return ec;
        }
      }

      if (!chunked) {
        net::get_lowest_layer(*stream).expires_after(
            builder_.client_request_timeout_);
        tie(ec, _) = co_await boost::beast::http::async_read(
            *stream, buffer, *req_parser);
        if (ec) {
          log::debug("[{}] async_read failed: {}", name(), ec.message());
          co_return ec;
        }
      }

      auto res = co_await do_handler(
          connection_info { net::get_local_endpoint(*stream).address(),
                            net::get_local_endpoint(*stream).port(),
                            net::get_remote_endpoint(*stream).address(),
                            net::get_remote_endpoint(*stream).port(),
                            listen_ep.address(),
                            listen_ep.port() },
          method,
          std::move(target),
          std::move(fields),
          [&]() -> any_async_readable_stream {
            if (chunked) {
              return async_message_parser_stream(
                  stream,
                  std::move(req_parser),
                  std::move(buffer),
                  builder_.client_request_timeout_);
            }
            return async_readable_vector_stream(
                std::move(req_parser->get().body()));
          }());
      auto& body = res.body();
      if (body.is_chunked()) {
        auto r = response<empty_body>(res.status_code().value(), 11);
        res.fields().to(r);
        r.keep_alive(keep_alive);
        r.chunked(true);

        auto r_serializer
            = boost::beast::http::response_serializer<empty_body>(r);

        net::get_lowest_layer(*stream).expires_after(
            builder_.client_request_timeout_);
        tie(ec, _) = co_await async_write_header(*stream, r_serializer);
        if (ec) {
          log::debug(
              "[{}] async_write_header failed: {}", name(), ec.message());
          co_return ec;
        }

        if (auto exp = co_await async_write_each_chunk(
                *stream, body, builder_.client_request_timeout_);
            !exp) {
          log::debug(
              "[{}] async_write_each_chunk failed: {}", name(), ec.message());
          co_return exp.error();
        }
      } else {
        auto r
            = response<vector_body<std::byte>>(res.status_code().value(), 11);
        res.fields().to(r);
        auto data = co_await async_read_all<std::vector<std::byte>>(body);
        if (data) {
          if (!*data) {
            co_return (*data).error();
          }
          r.body() = std::move(**data);
        }
        r.keep_alive(keep_alive);
        r.prepare_payload();

        net::get_lowest_layer(*stream).expires_after(
            builder_.client_request_timeout_);
        tie(ec, _) = co_await async_write(*stream, std::move(r));
        if (ec) {
          log::debug("[{}] async_write failed: {}", name(), ec.message());
          co_return ec;
        }
      }

      if (!keep_alive) {
        break;
      }
    }

    co_return ec;
  }

  lazy<http_response> do_handler(connection_info connection_info,
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

    auto route = builder_.router_.try_find(method, req_url.value().path());
    if (!route) {
      co_return http_response(http::status::not_found)
          .set_field(http::field::content_type,
                     http::fields::content_type::plaintext())
          .set_body("request path is not found");
    }

    auto request
        = http_request(std::move(connection_info),
                       route_params(route->match(req_url->path()).value(),
                                    std::string(route->pattern())),
                       req_url->path(),
                       method,
                       query_map::from(req_url->params()),
                       std::move(fields),
                       std::move(body),
                       route->state_maps());
    auto context = http_context(request);
    co_return co_await route->operator()(context);
  }

  static const char* name() noexcept
  {
    return "fitoria.http_server";
  }

  builder builder_;
  std::vector<lazy<void>> tasks_;
};

}

FITORIA_NAMESPACE_END

#endif
