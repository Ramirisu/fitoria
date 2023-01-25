//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef FITORIA_WEB_HTTP_SERVER_HPP
#define FITORIA_WEB_HTTP_SERVER_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/core/net.hpp>
#include <fitoria/core/url.hpp>

#include <fitoria/log.hpp>

#include <fitoria/web/http_context.hpp>
#include <fitoria/web/http_request.hpp>
#include <fitoria/web/router.hpp>
#include <fitoria/web/scope.hpp>

FITORIA_NAMESPACE_BEGIN

class http_server {
  using router_type = router;

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

    builder& route(const route& route)
    {
      if (auto res = router_.try_insert(route); !res) {
#if !FITORIA_NO_EXCEPTIONS
        throw system_error(res.error());
#else
        std::terminate();
#endif
      }

      return *this;
    }

    builder& route(const scope& scope)
    {
      for (auto& route : scope.routes()) {
        this->route(route);
      }

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

  net::awaitable<void> async_run()
  {
    log::info("[{}] starting server", name());
    run_impl(co_await net::this_coro::executor);
    co_return;
  }

  http_response serve_http_request(std::string_view target,
                                   http_request request)
  {
    auto encode_target = [](std::string_view target) -> std::string {
      urls::url url;
      auto pos = target.find('?');
      if (pos != std::string::npos) {
        url.set_query(target.substr(pos + 1));
      }
      url.set_path(target.substr(0, pos));
      return std::string(url.encoded_target());
    };

    net::io_context ioc;
    auto response = net::co_spawn(
        ioc,
        do_handler(
            net::ip::tcp::endpoint(net::ip::make_address("127.0.0.1"), 0),
            net::ip::tcp::endpoint(net::ip::make_address("127.0.0.1"), 0),
            request.method(), encode_target(target), request.headers(),
            std::move(request.body())),
        net::use_future);
    ioc.run();
    return response.get();
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

  net::awaitable<net::acceptor>
  new_acceptor(net::ip::tcp::endpoint endpoint) const
  {
    auto acceptor = net::acceptor(co_await net::this_coro::executor);

    acceptor.open(endpoint.protocol());
    acceptor.set_option(net::socket_base::reuse_address(true));
    acceptor.bind(endpoint);
    acceptor.listen(builder_.max_listen_connections_);
    co_return acceptor;
  }

  net::awaitable<void> do_listen(net::ip::tcp::endpoint endpoint) const
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
                    do_session(net::tcp_stream(std::move(socket))),
                    builder_.exception_handler_);
    }
  }

#if defined(FITORIA_HAS_OPENSSL)
  net::awaitable<void> do_listen(net::ip::tcp::endpoint endpoint,
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
                    do_session(net::ssl_stream(std::move(socket), ssl_ctx)),
                    builder_.exception_handler_);
    }
  }
#endif

  net::awaitable<void> do_session(net::tcp_stream stream) const
  {
    auto ec = co_await do_session_impl(stream);
    if (ec) {
      builder_.handle_network_error(ec);
      co_return;
    }

    stream.socket().shutdown(net::ip::tcp::socket::shutdown_send, ec);
  }

#if defined(FITORIA_HAS_OPENSSL)
  net::awaitable<void> do_session(net::ssl_stream stream) const
  {
    using std::tie;

    net::error_code ec;

    net::get_lowest_layer(stream).expires_after(
        builder_.client_request_timeout_);
    tie(ec) = co_await stream.async_handshake(net::ssl::stream_base::server);
    if (ec) {
      log::debug("[{}] async_handshake failed: {}", name(), ec.message());
      builder_.handle_network_error(ec);
      co_return;
    }

    if (ec = co_await do_session_impl(stream); ec) {
      builder_.handle_network_error(ec);
      co_return;
    }

    tie(ec) = co_await stream.async_shutdown();
    if (ec) {
      log::debug("[{}] async_shutdown failed: {}", name(), ec.message());
      builder_.handle_network_error(ec);
      co_return;
    }
  }
#endif

  template <typename Stream>
  net::awaitable<net::error_code> do_session_impl(Stream& stream) const
  {
    using std::tie;
    auto _ = std::ignore;

    net::flat_buffer buffer;
    net::error_code ec;

    for (;;) {
      http::detail::request_parser<http::detail::string_body> req_parser;

      net::get_lowest_layer(stream).expires_after(
          builder_.client_request_timeout_);
      tie(ec, _) = co_await http::detail::async_read_header(stream, buffer,
                                                            req_parser);
      if (ec) {
        if (ec != http::detail::error::end_of_stream) {
          log::debug("[{}] async_read_header failed: {}", name(), ec.message());
        }
        co_return ec;
      }

      auto& req = req_parser.get();
      bool keep_alive = req.keep_alive();

      if (auto it = req.find(http::field::expect);
          it != req.end() && it->value() == "100-continue") {
        net::get_lowest_layer(stream).expires_after(
            builder_.client_request_timeout_);
        tie(ec, _) = co_await http::detail::async_write(
            stream,
            static_cast<http::detail::response<http::detail::string_body>>(
                http_response(http::status::continue_)));
        if (ec) {
          log::debug("[{}] async_write 100-continue failed: {}", name(),
                     ec.message());
          co_return ec;
        }
      }

      net::get_lowest_layer(stream).expires_after(
          builder_.client_request_timeout_);
      tie(ec, _)
          = co_await http::detail::async_read(stream, buffer, req_parser);
      if (ec) {
        log::debug("[{}] async_read failed: {}", name(), ec.message());
        co_return ec;
      }

      auto res = static_cast<http::detail::response<http::detail::string_body>>(
          co_await do_handler(
              net::get_lowest_layer(stream).socket().local_endpoint(),
              net::get_lowest_layer(stream).socket().remote_endpoint(),
              req.method(), req.target(), to_header(req),
              std::move(req.body())));
      res.keep_alive(keep_alive);
      res.prepare_payload();

      net::get_lowest_layer(stream).expires_after(
          builder_.client_request_timeout_);
      tie(ec, _) = co_await http::detail::async_write(stream, std::move(res));
      if (ec) {
        log::debug("[{}] async_write failed: {}", name(), ec.message());
        co_return ec;
      }

      if (!keep_alive) {
        break;
      }
    }

    co_return ec;
  }

  net::awaitable<http_response>
  do_handler(net::ip::tcp::endpoint local_endpoint,
             net::ip::tcp::endpoint remote_endpoint,
             http::verb method,
             std::string target,
             http_header header,
             std::string body) const
  {
    auto req_url = urls::parse_origin_form(target);
    if (!req_url) {
      co_return http_response(http::status::bad_request)
          .set_header(http::field::content_type,
                      http::fields::content_type::plaintext())
          .set_body("request target is invalid");
    }

    auto route = builder_.router_.try_find(method, req_url.value().path());
    if (!route) {
      co_return http_response(http::status::not_found)
          .set_header(http::field::content_type,
                      http::fields::content_type::plaintext())
          .set_body("request path is not found");
    }

    auto request
        = http_request(local_endpoint, remote_endpoint,
                       route_params(segments_view::parse_param_map(
                                        route->path(), req_url->path())
                                        .value(),
                                    std::string(route->path())),
                       req_url->path(), method, to_query_map(req_url->params()),
                       std::move(header), std::move(body));
    auto context = http_context(
        http_context::invoker_type(route->middlewares(), route->handler()),
        request);
    co_return co_await context.next();
  }

  static query_map to_query_map(urls::params_view params)
  {
    query_map query;
    for (auto param : params) {
      if (param.has_value) {
        query[param.key] = param.value;
      }
    }

    return query;
  }

  static http_header
  to_header(const http::detail::request<http::detail::string_body>& req)
  {
    http_header header;
    for (auto& kv : req.base()) {
      header.set(kv.name(), kv.value());
    }
    return header;
  }

  static const char* name() noexcept
  {
    return "fitoria.http_server";
  }

  builder builder_;
  std::vector<net::awaitable<void>> tasks_;
};

FITORIA_NAMESPACE_END

#endif
