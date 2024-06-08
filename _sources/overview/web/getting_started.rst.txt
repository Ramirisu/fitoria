********************************************************************************
Getting Started
********************************************************************************

The following example demonstrates how to create a simple ``http_server`` and attach handlers to it.

`Getting Started Example <https://github.com/Ramirisu/fitoria/blob/main/example/web/basic/getting_started.cpp>`_

.. code-block:: cpp

   #include <fitoria/web.hpp>
   
   using namespace fitoria;
   using namespace fitoria::web;
   
   auto hello_world() -> awaitable<response>
   {
     co_return response::ok()
         .set_header(http::field::content_type, mime::text_plain())
         .set_body("Hello World!");
   }
   
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
                       .serve(route::get<"/">(hello_world))
                       .serve(route::post<"/echo">(echo))
                       .build();
   
     server.bind("127.0.0.1", 8080);
   
     ioc.run();
   }
