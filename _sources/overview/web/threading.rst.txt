********************************************************************************
Threading
********************************************************************************

Use ``net::static_thread_pool`` as the execution context to enable multi-threading for the ``http_server``. 

`Thread Pool Example <https://github.com/Ramirisu/fitoria/blob/main/example/web/basic/thread_pool.cpp>`_

.. code-block:: cpp

   int main()
   {
     auto tp = net::static_thread_pool();
     auto server
         = http_server::builder(tp.get_executor())
               .serve(
                   route::post<"/">([](std::string body) -> awaitable<response> {
                     co_return response::ok()
                         .set_header(http::field::content_type, mime::text_plain())
                         .set_body(body);
                   }))
               .build();
     server.bind("127.0.0.1", 8080);
   
     tp.join();
   }
