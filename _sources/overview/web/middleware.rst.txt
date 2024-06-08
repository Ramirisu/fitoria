********************************************************************************
Middleware
********************************************************************************

fitoria provides middlewares which allow users to modify ``reqeust`` and ``response`` before/after the handlers being invoked.

.. note:: 

   To create a custom middleware, implement customization point ``to_middleware_t``.

.. code-block:: cpp

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


fitoria also provides built-in middlewares for convenience.

* ``middleware::logger``
* ``middleware::exception_handler``
* ``middleware::decompress`` (``deflate``, ``gzip`` and ``brotli``)

.. note:: 

  ``gzip`` and ``brotli`` must be installed in order to make them functional in ``middleware::decompress``.


Middlewares can be mounted by ``scope::use(Middleware&&)`` and ``route::use(Middleware&&)``.

.. code-block:: cpp

   int main()
   {
     auto ioc = net::io_context();
     auto server = http_server::builder(ioc)
                       .serve(scope()
                                  .use(middleware::logger())
                                  .use(middleware::exception_handler())
                                  .use(middleware::decompress())
                                  .use(trace_id())
                                  .serve(route::post<"/">(echo)))
                       .build();
     server.bind("127.0.0.1", 8080);
   
     ioc.run();
   }
