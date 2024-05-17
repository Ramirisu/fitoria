.. _middleware:

********************************************************************************
Middleware
********************************************************************************

fitoria provides middlewares which allow users to modify ``reqeust`` and ``response`` before/after the handlers being invoked.

.. note:: 

   To create a custom middleware, implement customization point ``to_middleware_t``.

.. code-block:: cpp

   template <typename Request, typename Response, typename Next>
   class my_log_middleware {
     friend class my_log;
   
   public:
     awaitable<response> operator()(request& req) const
     {
       std::cout << fmt::format("{} {} HTTP/{}",
                                std::string(to_string(req.method())),
                                req.path().match_path(),
                                req.version())
                 << std::endl;
   
       response res = co_await next_(req);
   
       std::cout << fmt::format(
           "HTTP/{} {} {}",
           http::version::v1_1,
           to_underlying(res.status_code().value()),
           std::string(obsolete_reason(res.status_code().value())))
                 << std::endl;
   
       co_return res;
     }
   
   private:
     template <typename Next2>
     my_log_middleware(Next2 next, log::level lv)
         : next_(std::forward<Next2>(next))
         , lv_(lv)
     {
     }
   
     Next next_;
     log::level lv_;
   };
   
   class my_log {
   public:
     my_log(log::level lv)
         : lv_(lv)
     {
     }
   
     template <typename Request,
               typename Response,
               decay_to<my_log> Self,
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
       return my_log_middleware<Request, Response, std::decay_t<Next>>(
           std::forward<Next>(next), lv_);
     }
   
     log::level lv_;
   };

fitoria also provides built-in middlewares for convenience.

* ``middleware::logger``
* ``middleware::exception_handler``
* ``middleware::decompress`` (``deflate``, ``gzip`` and ``brotli``)

.. note:: 

  ``gzip`` and ``brotli`` must be installed for enabling their functionality in ``middleware::decompress``.


Middlewares can be mounted by ``scope::use(Middleware&&)`` and ``route::use(Middleware&&)``.

.. code-block:: cpp

   int main()
   {
     auto ioc = net::io_context();
     auto server = http_server::builder(ioc)
                       .serve(scope<"/api/v1">()
                                  .use(middleware::logger())
                                  .use(middleware::exception_handler())
                                  .use(middleware::decompress())
                                  .use(my_log(log::level::info))
                                  .serve(route::get<"/users/{user}">(get_user)))
                       .build();
     server.bind("127.0.0.1", 8080);
   
     ioc.run();
   }
