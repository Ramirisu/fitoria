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

FITORIA_NAMESPACE_BEGIN

class http_server {
public:
  using handler_trait = detail::handler_trait;
  using handlers_invoker_type = detail::handlers_invoker<handler_trait>;

  http_server(unsigned int nb_threads = std::thread::hardware_concurrency())
      : nb_threads_(std::max(nb_threads, 1U))
      , running_(false)
      , ioc_(nb_threads_)
      , thread_pool_(nb_threads_)
  {
  }

  ~http_server()
  {
    stop();
  }

  expected<void, router_error> route(const router& router)
  {
    return router_tree_.try_insert(router);
  }

  expected<void, router_error> route(const router_group& router_group)
  {
    for (auto&& router : router_group.get_all_routers()) {
      if (auto res = router_tree_.try_insert(router); !res) {
        return res;
      }
    }

    return {};
  }

  void run(std::string_view addr, std::uint16_t port)
  {
    bool running = running_.exchange(true);

    net::co_spawn(
        ioc_,
        do_listen(net::ip::tcp::endpoint(net::ip::make_address(addr), port)),
        net::detached);

    if (!running) {
      start_worker_threads();
    }
  }

  void run_with_tls(std::string_view addr,
                    std::uint16_t port,
                    net::ssl::context ssl_ctx)
  {
    bool running = running_.exchange(true);

    net::co_spawn(
        ioc_,
        do_listen(net::ip::tcp::endpoint(net::ip::make_address(addr), port),
                  std::move(ssl_ctx)),
        net::detached);

    if (!running) {
      start_worker_threads();
    }
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
  void start_worker_threads()
  {
    for (auto i = 0U; i < nb_threads_; ++i) {
      net::post(thread_pool_, [&]() { ioc_.run(); });
    }
  }

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
    acceptor.listen(net::socket_base::max_listen_connections);
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

  net::awaitable<void> do_session(net::tcp_stream stream)
  {
    co_await do_session_impl(stream);

    net::error_code ec;
    stream.socket().shutdown(net::ip::tcp::socket::shutdown_send, ec);
  }

  net::awaitable<void> do_session(net::ssl_stream stream)
  {
    net::get_lowest_layer(stream).expires_after(std::chrono::seconds(30));

    co_await stream.async_handshake(net::ssl::stream_base::server);

    co_await do_session_impl(stream.next_layer());

    net::error_code ec;
    net::get_lowest_layer(stream).socket().shutdown(
        net::ip::tcp::socket::shutdown_send, ec);
  }

  net::awaitable<void> do_session_impl(net::tcp_stream& stream)
  {
    net::flat_buffer buffer;

    for (;;) {
      try {
        stream.expires_after(std::chrono::seconds(30));

        http::request<http::string_body> req;
        co_await http::async_read(stream, buffer, req);

        bool keep_alive = req.keep_alive();

        http::response<http::string_body> res;
        res.keep_alive(keep_alive);

        co_await do_handler(req, res);

        res.prepare_payload();

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
          = router_tree_.try_find(req.method(), req_url.value().path());
          router) {
        if (auto qs = convert_route_param_to_query_string(
                router.value().path(), req_url.value().encoded_path());
            qs) {
          http_request request(req, req_url.value());
          http_context ctx(handlers_invoker_type(router.value().handlers()),
                           router.value().path(),
                           urls::parse_query(qs.value()).value(), request);
          co_await ctx.start();
          // TODO: http response
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

  /// @brief convert parameterized path and path into name-value pairs for being
  /// consumed by urls::parse_query()
  /// @param router_path "/api/v1/users/:user/repos/:repo"
  /// @param req_path "/api/v1/users/ramirisu/repos/fitoria"
  /// @return "user=ramirisu&repo=fitoria"
  static auto
  convert_route_param_to_query_string(std::string_view router_path,
                                      std::string_view req_path) noexcept
      -> optional<std::string>
  {
    auto split = [](std::string_view p) noexcept
        -> std::tuple<std::string_view, std::string_view> {
      p.remove_prefix(1);
      auto pos = p.find('/');
      if (pos == std::string_view::npos) {
        return { p.substr(0, pos), {} };
      }
      return { p.substr(0, pos), p.substr(pos) };
    };

    std::string qs;

    while (!router_path.empty() && !req_path.empty()) {
      std::string_view router_path_token;
      std::string_view req_path_token;
      std::tie(router_path_token, router_path) = split(router_path);
      std::tie(req_path_token, req_path) = split(req_path);
      if (!router_path_token.starts_with(':')) {
        continue;
      }
      router_path_token.remove_prefix(1);
      qs += router_path_token;
      qs += '=';
      qs += req_path_token;
      qs += '&';
    }

    if (qs.ends_with('&')) {
      qs.pop_back();
    }

    if (!router_path.empty() || !req_path.empty()) {
      return nullopt;
    }

    return qs;
  }

  unsigned int nb_threads_;
  std::atomic<bool> running_;
  net::io_context ioc_;
  net::thread_pool thread_pool_;
  router_tree router_tree_;
};

FITORIA_NAMESPACE_END
