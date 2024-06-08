//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria/web.hpp>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

using namespace fitoria;
using namespace fitoria::web;

template <typename Request, typename Response, typename Next>
class trace_id_middleware {
  friend class trace_id;

public:
  awaitable<response> operator()(request& req) const
  {
    auto id
        = req.header().get("X-Trace-Id").transform([](auto id) -> std::string {
            return std::string(id);
          });

    response res = co_await next_(req);

    if (!res.header().contains("X-Trace-Id")) {
      boost::uuids::random_generator gen;
      co_return res.builder()
          .set_header("X-Trace-Id", id.value_or(to_string(gen())))
          .build();
    }

    co_return res;
  }

private:
  template <typename Next2>
  trace_id_middleware(Next2 next)
      : next_(std::forward<Next2>(next))
  {
  }

  Next next_;
};

class trace_id {
public:
  template <typename Request,
            typename Response,
            decay_to<trace_id> Self,
            typename Next>
  friend auto
  tag_invoke(to_middleware_t<Request, Response>, Self&& self, Next&& next)
  {
    return std::forward<Self>(self)
        .template to_middleware_impl<Request, Response>(
            std::forward<Next>(next));
  }

private:
  template <typename Request, typename Response, typename Next>
  auto to_middleware_impl(Next&& next) const
  {
    return trace_id_middleware<Request, Response, std::decay_t<Next>>(
        std::forward<Next>(next));
  }
};

auto echo(std::string body) -> awaitable<response>
{
  co_return response::ok()
      .set_header(http::field::content_type, mime::text_plain())
      .set_body(body);
}

int main()
{
  auto ioc = net::io_context();
  auto server = http_server::builder(ioc)
                    .serve(scope()
                               .use(middleware::logger())
#if !FITORIA_NO_EXCEPTIONS
                               .use(middleware::exception_handler())
#endif
                               .use(middleware::decompress())
                               .use(trace_id())
                               .serve(route::post<"/">(echo)))
                    .build();
  server.bind("127.0.0.1", 8080);

  ioc.run();
}
