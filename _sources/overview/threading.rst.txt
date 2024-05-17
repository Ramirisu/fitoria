.. _threading:

********************************************************************************
Threading
********************************************************************************

``net::io_context::run()`` can be called by multiple threads simutaneously to enable threading. In the following example we create a ``net::static_thread_pool`` to run the ``net::io_context``. (`Thread Pool Example <https://github.com/Ramirisu/fitoria/blob/main/example/web/thread_pool.cpp>`_)

.. code-block:: cpp

   int main()
   {
     auto ioc = net::io_context();
     auto server
         = http_server::builder(ioc)
               .serve(route::get<"/">([](std::string body) -> awaitable<response> {
                 co_return response::ok()
                     .set_header(http::field::content_type,
                                 http::fields::content_type::plaintext())
                     .set_body(body);
               }))
               .build();
     server.bind("127.0.0.1", 8080);
   
     const std::size_t threads = std::thread::hardware_concurrency();
     auto tp = net::static_thread_pool(threads);
     for (std::size_t i = 0; i < threads; ++i) {
       net::post(ioc, [&]() { ioc.run(); });
     }
     ioc.run();
   }
