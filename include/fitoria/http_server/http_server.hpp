//
// Copyright (c) 2022 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include <fitoria/core/config.hpp>

#include <fitoria/core/net.hpp>
#include <fitoria/core/url.hpp>
#include <fitoria/core/utility.hpp>
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

  http_server_config& set_threads(std::uint32_t num)
  {
    threads_ = std::max(num, 1U);
    return *this;
  }

  http_server_config& set_max_listen_connections(int num)
  {
    max_listen_connections_ = num;
    return *this;
  }

  http_server_config&
  set_client_request_timeout(std::chrono::milliseconds timeout)
  {
    client_request_timeout_ = timeout;
    return *this;
  }

  http_server_config& route(const router& router)
  {
    if (auto res = router_tree_.try_insert(router); !res) {
      throw res.error();
    }

    return *this;
  }

  http_server_config& route(const router_group& router_group)
  {
    for (auto&& router : router_group.get_all_routers()) {
      if (auto res = router_tree_.try_insert(router); !res) {
        throw res.error();
      }
    }

    return *this;
  }

private:
  std::uint32_t threads_ = std::thread::hardware_concurrency();
  int max_listen_connections_ = net::socket_base::max_listen_connections;
  std::chrono::milliseconds client_request_timeout_ = std::chrono::seconds(5);
  router_tree router_tree_;
};

class http_server {
public:
  using handler_trait = detail::handler_trait;
  using handlers_invoker_type = detail::handlers_invoker<handler_trait>;

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
        net::detached);

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
        net::detached);

    return *this;
  }
#endif

  http_server& run()
  {
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
    ioc_.stop();
  }

private:
  using acceptor_t = typename net::ip::tcp::acceptor::template rebind_executor<
      net::use_awaitable_t<>::executor_with_default<net::any_io_executor>>::
      other;

  net::awaitable<acceptor_t> new_acceptor(net::ip::tcp::endpoint endpoint)
  {
    auto acceptor = net::use_awaitable.as_default_on(
        net::ip::tcp::acceptor(co_await net::this_coro::executor));

    acceptor.open(endpoint.protocol());
    acceptor.set_option(net::socket_base::reuse_address(true));
    acceptor.bind(endpoint);
    acceptor.listen(config_.max_listen_connections_);
    co_return acceptor;
  }

  net::awaitable<void> do_listen(net::ip::tcp::endpoint endpoint)
  {
    auto acceptor = co_await new_acceptor(endpoint);

    for (;;) {
      net::co_spawn(
          acceptor.get_executor(),
          do_session(net::tcp_stream(co_await acceptor.async_accept())),
          net::detached);
    }
  }

#if defined(FITORIA_HAS_OPENSSL)
  net::awaitable<void> do_listen(net::ip::tcp::endpoint endpoint,
                                 net::ssl::context ssl_ctx)
  {
    auto acceptor = co_await new_acceptor(endpoint);

    for (;;) {
      net::co_spawn(acceptor.get_executor(),
                    do_session(net::ssl_stream(co_await acceptor.async_accept(),
                                               ssl_ctx)),
                    net::detached);
    }
  }
#endif

  net::awaitable<void> do_session(net::tcp_stream stream)
  {
    co_await do_session_impl(stream);

    net::error_code ec;
    stream.socket().shutdown(net::ip::tcp::socket::shutdown_send, ec);
  }

#if defined(FITORIA_HAS_OPENSSL)
  net::awaitable<void> do_session(net::ssl_stream stream)
  {
    net::get_lowest_layer(stream).expires_after(
        config_.client_request_timeout_);

    co_await stream.async_handshake(net::ssl::stream_base::server);

    co_await do_session_impl(stream.next_layer());

    net::error_code ec;
    net::get_lowest_layer(stream).socket().shutdown(
        net::ip::tcp::socket::shutdown_send, ec);
  }
#endif

  net::awaitable<void> do_session_impl(net::tcp_stream& stream)
  {
    net::flat_buffer buffer;

    for (;;) {
      try {
        stream.expires_after(config_.client_request_timeout_);

        http::request<http::string_body> req;
        co_await http::async_read(stream, buffer, req);

        bool keep_alive = req.keep_alive();

        http::response<http::string_body> res;
        res.keep_alive(keep_alive);

        co_await do_handler(req, res);

        res.prepare_payload();

        stream.expires_after(config_.client_request_timeout_);

        co_await net::async_write(stream,
                                  http::message_generator(std::move(res)),
                                  net::use_awaitable);

        if (!keep_alive) {
          break;
        }
      } catch (boost::system::system_error& ec) {
        if (ec.code() != http::error::end_of_stream)
          throw;
      }
    }
  }

  net::awaitable<void> do_handler(http::request<http::string_body>& req,
                                  http::response<http::string_body>& res)
  {
    if (auto req_url = urls::parse_origin_form(req.target()); req_url) {
      if (auto router
          = config_.router_tree_.try_find(req.method(), req_url.value().path());
          router) {
        if (auto qs = convert_route_param_to_query_string(
                router.value().path(), req_url.value().encoded_path());
            qs) {
          http_request request(req, req_url.value());
          http_context ctx(handlers_invoker_type(router.value().handlers()),
                           router.value().path(),
                           urls::parse_query(qs.value()).value(), request, res);
          co_await ctx.start();
        } else {
          res.result(http::status::bad_request);
        }
      } else {
        res.result(http::status::not_found);
      }
    } else {
      res.result(http::status::bad_request);
    }
  }

  /// @brief convert parameterized path and path into name-value pairs for
  /// being consumed by urls::parse_query()
  /// @param router_path "/api/v1/users/{user}/repos/{repo}"
  /// @param req_path "/api/v1/users/ramirisu/repos/fitoria"
  /// @return "user=ramirisu&repo=fitoria"
  static auto
  convert_route_param_to_query_string(std::string_view router_path,
                                      std::string_view req_path) noexcept
      -> expected<std::string, router_error>
  {
    auto router_segs = route::to_segments(router_path);
    auto req_segs = route::to_segments(req_path);

    if (!router_segs || !req_segs || router_segs->size() != req_segs->size()) {
      return unexpected<router_error>(router_error::parse_path_error);
    }

    std::string qs;

    for (std::size_t i = 0; i < router_segs->size(); ++i) {
      if (router_segs.value()[i].is_param) {
        qs += router_segs.value()[i].escaped;
        qs += '=';
        qs += req_segs.value()[i].original;
        qs += '&';
      }
    }

    if (qs.ends_with('&')) {
      qs.pop_back();
    }

    return qs;
  }

  http_server_config config_;
  net::io_context ioc_;
  net::thread_pool thread_pool_;
};

FITORIA_NAMESPACE_END
