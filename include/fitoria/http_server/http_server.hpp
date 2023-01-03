//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include <fitoria/core/config.hpp>

#include <fitoria/core/log.hpp>
#include <fitoria/core/net.hpp>
#include <fitoria/core/url.hpp>

#include <fitoria/http_server/http_context.hpp>
#include <fitoria/http_server/http_request.hpp>
#include <fitoria/http_server/router_group.hpp>
#include <fitoria/http_server/router_tree.hpp>

#include <thread>

FITORIA_NAMESPACE_BEGIN

class http_server_config {
  friend class http_server;

public:
  template <typename F>
  http_server_config& configure(F&& f)
    requires std::invocable<F, http_server_config&>
  {
    std::invoke(std::forward<F>(f), *this);
    return *this;
  }

  http_server_config& set_threads(std::uint32_t num) noexcept
  {
    threads_ = std::max(num, 1U);
    return *this;
  }

  http_server_config& set_max_listen_connections(int num) noexcept
  {
    max_listen_connections_ = num;
    return *this;
  }

  http_server_config&
  set_client_request_timeout(std::chrono::milliseconds timeout) noexcept
  {
    client_request_timeout_ = timeout;
    return *this;
  }

  template <typename F>
  http_server_config& set_exception_handler(F&& f)
    requires std::invocable<F, std::exception_ptr>
  {
    exception_handler_ = std::forward<F>(f);
    return *this;
  }

  http_server_config& route(const router& router)
  {
    if (auto res = router_tree_.try_insert(router); !res) {
      throw system_error(res.error());
    }

    return *this;
  }

  http_server_config& route(const router_group& router_group)
  {
    for (auto& router : router_group.routers()) {
      if (auto res = router_tree_.try_insert(router); !res) {
        throw system_error(res.error());
      }
    }

    return *this;
  }

private:
  static void default_exception_handler(std::exception_ptr ptr)
  {
    if (ptr) {
      try {
        std::rethrow_exception(ptr);
      } catch (const std::exception& ex) {
        log::debug(name(), "exception: {}", ex.what());
      }
    }
  }

  static const char* name() noexcept
  {
    return "fitoria.http_server_config";
  }

  std::uint32_t threads_ = std::thread::hardware_concurrency();
  int max_listen_connections_ = net::socket_base::max_listen_connections;
  std::chrono::milliseconds client_request_timeout_ = std::chrono::seconds(5);
  std::function<void(std::exception_ptr)> exception_handler_
      = default_exception_handler;
  router_tree router_tree_;
};

class http_server {
public:
  using handler_trait = http_handler_trait;
  using handlers_invoker_type = handlers_invoker<handler_trait>;
  using execution_context = net::io_context;

  http_server(http_server_config config)
      : config_(std::move(config))
      , ioc_(static_cast<int>(config_.threads_))
      , thread_pool_(config_.threads_)
  {
  }

  ~http_server()
  {
    stop();
  }

  http_server& bind(std::string_view addr, std::uint16_t port)
  {
    net::co_spawn(
        ioc_,
        do_listen(net::ip::tcp::endpoint(net::ip::make_address(addr), port)),
        config_.exception_handler_);

    return *this;
  }

#if defined(FITORIA_HAS_OPENSSL)
  http_server&
  bind_ssl(std::string_view addr, std::uint16_t port, net::ssl::context ssl_ctx)
  {
    net::co_spawn(
        ioc_,
        do_listen(net::ip::tcp::endpoint(net::ip::make_address(addr), port),
                  std::move(ssl_ctx)),
        config_.exception_handler_);

    return *this;
  }
#endif

  http_server& run()
  {
    log::info(name(), "starting with {} workers", config_.threads_);
    for (auto i = 0U; i < config_.threads_; ++i) {
      net::post(thread_pool_, [&]() { ioc_.run(); });
    }

    return *this;
  }

  void wait()
  {
    ioc_.run();
  }

  void stop()
  {
    log::info(name(), "stopping workers");
    ioc_.stop();
  }

  execution_context& get_execution_context()
  {
    return ioc_;
  }

private:
  using native_request_t = http::request<http::string_body>;
  using native_response_t = http::response<http::string_body>;

  net::awaitable<net::accepter>
  new_acceptor(net::ip::tcp::endpoint endpoint) const
  {
    auto acceptor = net::accepter(co_await net::this_coro::executor);

    acceptor.open(endpoint.protocol());
    acceptor.set_option(net::socket_base::reuse_address(true));
    acceptor.bind(endpoint);
    acceptor.listen(config_.max_listen_connections_);
    co_return acceptor;
  }

  net::awaitable<void> do_listen(net::ip::tcp::endpoint endpoint) const
  {
    auto acceptor = co_await new_acceptor(endpoint);

    for (;;) {
      auto [ec, socket] = co_await acceptor.async_accept();
      if (ec) {
        log::debug(name(), "async_accept failed: {}", ec.message());
        continue;
      }

      net::co_spawn(acceptor.get_executor(),
                    do_session(net::tcp_stream(std::move(socket))),
                    net::detached);
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
        log::debug(name(), "async_accept failed: {}", ec.message());
        continue;
      }

      net::co_spawn(acceptor.get_executor(),
                    do_session(net::ssl_stream(std::move(socket), ssl_ctx)),
                    net::detached);
    }
  }
#endif

  net::awaitable<void> do_session(net::tcp_stream stream) const
  {
    auto ec = co_await do_session_impl(stream);
    if (ec) {
      co_return;
    }

    stream.socket().shutdown(net::ip::tcp::socket::shutdown_send, ec);
  }

#if defined(FITORIA_HAS_OPENSSL)
  net::awaitable<void> do_session(net::ssl_stream stream) const
  {
    net::get_lowest_layer(stream).expires_after(
        config_.client_request_timeout_);

    auto [ec] = co_await stream.async_handshake(net::ssl::stream_base::server);
    if (ec) {
      log::debug(name(), "async_handshake failed: {}", ec.message());
      co_return;
    }

    if (ec = co_await do_session_impl(stream); ec) {
      co_return;
    }

    std::tie(ec) = co_await stream.async_shutdown();
    if (ec) {
      log::debug(name(), "async_shutdown failed: {}", ec.message());
    }
  }
#endif

  template <typename Stream>
  net::awaitable<net::error_code> do_session_impl(Stream& stream) const
  {
    net::flat_buffer buffer;
    net::error_code ec;

    for (;;) {
      net::get_lowest_layer(stream).expires_after(
          config_.client_request_timeout_);

      native_request_t req;
      std::tie(ec, std::ignore)
          = co_await http::async_read(stream, buffer, req);
      if (ec) {
        if (ec != http::error::end_of_stream) {
          log::debug(name(), "async_read failed: {}", ec.message());
        }
        co_return ec;
      }

      bool keep_alive = req.keep_alive();

      auto res = co_await do_handler(
          net::get_lowest_layer(stream).socket().remote_endpoint(), req);
      res.keep_alive(keep_alive);
      res.prepare_payload();

      net::get_lowest_layer(stream).expires_after(
          config_.client_request_timeout_);

      std::tie(ec, std::ignore) = co_await net::async_write(
          stream, http::message_generator(std::move(res)));
      if (ec) {
        log::debug(name(), "async_write failed: {}", ec.message());
        co_return ec;
      }

      if (!keep_alive) {
        break;
      }
    }

    co_return ec;
  }

  net::awaitable<native_response_t>
  do_handler(net::ip::tcp::endpoint remote_endpoint,
             native_request_t& req) const
  {
    auto req_url = urls::parse_origin_form(req.target());
    if (!req_url) {
      co_return http_response(http::status::bad_request);
    }

    auto router
        = config_.router_tree_.try_find(req.method(), req_url.value().path());
    if (!router) {
      co_return http_response(http::status::not_found);
    }

    auto route_params = route::parse_param_map(router->path(), req_url->path());
    if (!route_params) {
      co_return http_response(http::status::bad_request);
    }

    auto request = http_request(
        remote_endpoint, http_route(*route_params, std::string(router->path())),
        req_url->path(), req.method(), req_url->query(),
        to_query_map(req_url->params()), to_header(req), std::move(req.body()));
    auto context = http_context(handlers_invoker<handler_trait>(
                                    router->middlewares(), router->handler()),
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

  static http_header to_header(const native_request_t& req)
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

  http_server_config config_;
  execution_context ioc_;
  net::thread_pool thread_pool_;
};

FITORIA_NAMESPACE_END
