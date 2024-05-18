********************************************************************************
Method
********************************************************************************

Register a handler that serves specific method defined under ``enum class http::verb::*`` by calling ``route::handle(...)``, or simply use ``route::get(...)``, ``route::post(...)``, ``route::put(...)``, ``route::patch(...)``, ``route::delete_(...)``, ``route::head(...)``, ``route::options(...)`` for convenience. ``route::any(...)`` can be used for registering a handler that serves any method. (`Method Example <https://github.com/Ramirisu/fitoria/blob/main/example/web/method.cpp>`_)

.. code-block:: cpp

   int main()
   {
     auto ioc = net::io_context();
     auto server = http_server::builder(ioc)
                       .serve(route::handle<"/">(http::verb::get, get_handler))
                       .serve(route::get<"/get">(get_handler))
                       .serve(route::post<"/post">(post_handler))
                       .serve(route::put<"/put">(put_handler))
                       .serve(route::patch<"/patch">(patch_handler))
                       .serve(route::delete_<"/delete">(delete_handler))
                       .serve(route::head<"/head">(head_handler))
                       .serve(route::options<"/options">(options_handler))
                       .serve(route::any<"/any">(any_handler))
                       .build();
     server.bind("127.0.0.1", 8080);
     ioc.run();
   }
