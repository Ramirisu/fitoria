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

#include <fitoria/web/handler.hpp>
#include <fitoria/web/http_context.hpp>
#include <fitoria/web/http_response.hpp>
#include <fitoria/web/mock_http_request.hpp>
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

  http_response serve_http_request(std::string_view path, mock_http_request req)
  {
    boost::urls::url url;
    url.set_path(path);
    url.set_query(req.query().to_string());

    return net::sync_wait(
        do_handler(connection_info { net::ip::make_address("127.0.0.1"),
                                     0,
                                     net::ip::make_address("127.0.0.1"),
                                     0,
                                     net::ip::make_address("127.0.0.1"),
                                     0 },
                   req.method(),
                   std::string(url.encoded_target()),
                   req.fields(),
                   async_readable_vector_stream(
                       std::span(req.body().begin(), req.body().end()))));
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

      net::co_spawn(acceptor.get_executor(),
                    do_session(net::tcp_stream(std::move(socket)), endpoint),
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

      net::co_spawn(
          acceptor.get_executor(),
          do_session(net::ssl_stream(std::move(socket), ssl_ctx), endpoint),
          builder_.exception_handler_);
    }
  }
#endif

  lazy<void> do_session(net::tcp_stream stream,
                        net::ip::tcp::endpoint listen_ep) const
  {
    auto ec = co_await do_session_impl(stream, std::move(listen_ep));
    if (ec) {
      builder_.handle_network_error(ec);
      co_return;
    }

    stream.socket().shutdown(net::ip::tcp::socket::shutdown_send, ec);
  }

#if defined(FITORIA_HAS_OPENSSL)
  lazy<void> do_session(net::ssl_stream stream,
                        net::ip::tcp::endpoint listen_ep) const
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

    if (ec = co_await do_session_impl(stream, std::move(listen_ep)); ec) {
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
  lazy<net::error_code> do_session_impl(Stream& stream,
                                        net::ip::tcp::endpoint listen_ep) const
  {
    using boost::beast::http::empty_body;
    using boost::beast::http::request_parser;
    using boost::beast::http::response;
    using boost::beast::http::string_body;
    using boost::beast::http::vector_body;
    using std::tie;
    auto _ = std::ignore;

    net::flat_buffer buffer;
    net::error_code ec;

    for (;;) {
      request_parser<vector_body<std::byte>> req_parser;
      req_parser.body_limit(boost::none);

      net::get_lowest_layer(stream).expires_after(
          builder_.client_request_timeout_);
      tie(ec, _) = co_await async_read_header(stream, buffer, req_parser);
      if (ec) {
        if (ec != http::error::end_of_stream) {
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
        tie(ec, _)
            = co_await async_write(stream,
                                   static_cast<response<empty_body>>(
                                       http_response(http::status::continue_)));
        if (ec) {
          log::debug(
              "[{}] async_write 100-continue failed: {}", name(), ec.message());
          co_return ec;
        }
      }

      std::vector<std::byte> chunk;
      auto on_chunk_header
          = [&chunk](std::uint64_t size, auto, net::error_code&) {
              chunk.reserve(size);
              chunk.clear();
            };
      auto on_chunk_body
          = [&chunk](std::uint64_t remain, auto body, net::error_code& ec) {
              if (remain == body.size()) {
                ec = http::error::end_of_chunk;
              }
              auto s = std::as_bytes(std::span(body.data(), body.size()));
              chunk.insert(chunk.end(), s.begin(), s.end());

              return body.size();
            };

      if (req_parser.chunked()) {
        req_parser.on_chunk_header(on_chunk_header);
        req_parser.on_chunk_body(on_chunk_body);
      } else {
        net::get_lowest_layer(stream).expires_after(
            builder_.client_request_timeout_);
        tie(ec, _) = co_await boost::beast::http::async_read(
            stream, buffer, req_parser);
        if (ec) {
          log::debug("[{}] async_read failed: {}", name(), ec.message());
          co_return ec;
        }
      }

      auto res = static_cast<response<string_body>>(co_await do_handler(
          connection_info {
              net::get_lowest_layer(stream).socket().local_endpoint().address(),
              net::get_lowest_layer(stream).socket().local_endpoint().port(),
              net::get_lowest_layer(stream)
                  .socket()
                  .remote_endpoint()
                  .address(),
              net::get_lowest_layer(stream).socket().remote_endpoint().port(),
              listen_ep.address(),
              listen_ep.port() },
          req.method(),
          req.target(),
          to_http_fields(req),
          req.chunked()
              ? any_async_readable_stream(
                  async_readable_chunk_stream(stream,
                                              req_parser,
                                              buffer,
                                              chunk,
                                              builder_.client_request_timeout_))
              : any_async_readable_stream(
                  async_readable_vector_stream(std::move(req.body())))));
      res.keep_alive(keep_alive);
      res.prepare_payload();

      net::get_lowest_layer(stream).expires_after(
          builder_.client_request_timeout_);
      tie(ec, _) = co_await async_write(stream, std::move(res));
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
                       to_query_map(req_url->params()),
                       std::move(fields),
                       std::move(body),
                       route->state_maps());
    auto context = http_context(request);
    co_return co_await route->operator()(context);
  }

  static query_map to_query_map(boost::urls::params_view params)
  {
    query_map query;
    for (auto param : params) {
      if (param.has_value) {
        query[param.key] = param.value;
      }
    }

    return query;
  }

  static http_fields
  to_http_fields(const boost::beast::http::request<
                 boost::beast::http::vector_body<std::byte>>& req)
  {
    http_fields fields;
    for (auto& kv : req.base()) {
      fields.insert(kv.name(), kv.value());
    }
    return fields;
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
