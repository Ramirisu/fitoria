//
// Copyright (c) 2022 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include <fitoria/core/config.hpp>

#include <fitoria/core/net.hpp>
#include <fitoria/core/utility.hpp>
#include <fitoria/http_server/http_context.hpp>
#include <fitoria/router/router_group.hpp>
#include <fitoria/router/router_tree.hpp>

FITORIA_NAMESPACE_BEGIN

class http_server {
public:
  using router_group_type = router_group<handler_trait>;
  using router_tree_type = router_tree<handler_trait>;
  using handlers_invoker_type = detail::handlers_invoker<handler_trait>;

  http_server(unsigned int nb_threads = std::thread::hardware_concurrency())
      : nb_threads_(std::max(nb_threads, 1U))
      , running_(false)
      , ioc_(nb_threads_)
      , thread_pool_(nb_threads_)
  {
  }

  expected<void, router_error> route(const router_group_type& router_group)
  {
    for (auto&& routes : router_group.get_all_routers()) {
      if (auto res = router_tree_.try_insert(routes); !res) {
        return res;
      }
    }

    return {};
  }

  ~http_server()
  {
    stop();
  }

  void run(string_view addr, std::uint16_t port)
  {
    if (running_.exchange(true)) {
      return;
    }

    net::co_spawn(
        ioc_,
        do_listen(net::ip::tcp::endpoint(net::ip::make_address(addr), port)),
        net::detached);

    for (auto i = 0U; i < nb_threads_; ++i) {
      net::post(thread_pool_, [&]() { ioc_.run(); });
    }
  }

  void stop()
  {
    ioc_.stop();
  }

private:
  net::awaitable<void> do_session(net::tcp_stream stream)
  {
    net::flat_buffer buffer;

    for (;;) {
      try {
        stream.expires_after(std::chrono::seconds(30));

        http::request<http::string_body> req;
        co_await http::async_read(stream, buffer, req);

        bool keep_alive = req.keep_alive();

        auto router
            = router_tree_.try_find(req.method(), string_view(req.target()));
        http_context ctx(handlers_invoker_type(router.value().handlers()));
        co_await ctx.start();

        http::response<http::string_body> res;
        res.keep_alive(keep_alive);
        res.prepare_payload();

        co_await net::async_write(stream,
                                  http::message_generator(std::move(res)),
                                  net::use_awaitable);

        if (!keep_alive) {
          break;
        }
      } catch (boost::system::system_error& se) {
        if (se.code() != http::error::end_of_stream)
          throw;
      }
    }

    net::error_code ec;
    stream.socket().shutdown(net::ip::tcp::socket::shutdown_send, ec);
  }

  net::awaitable<void> do_listen(net::ip::tcp::endpoint endpoint)
  {
    auto acceptor = net::use_awaitable.as_default_on(
        net::ip::tcp::acceptor(co_await net::this_coro::executor));

    acceptor.open(endpoint.protocol());
    acceptor.set_option(net::socket_base::reuse_address(true));
    acceptor.bind(endpoint);
    acceptor.listen(net::socket_base::max_listen_connections);

    for (;;) {
      net::co_spawn(
          acceptor.get_executor(),
          do_session(net::tcp_stream(co_await acceptor.async_accept())),
          net::detached);
    }
  }

  unsigned int nb_threads_;
  std::atomic<bool> running_;
  net::io_context ioc_;
  net::thread_pool thread_pool_;
  router_tree<handler_trait> router_tree_;
};

FITORIA_NAMESPACE_END
