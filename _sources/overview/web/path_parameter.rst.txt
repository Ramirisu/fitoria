********************************************************************************
Path Parameter
********************************************************************************

Use ``request::path()`` to access the path parameters. 

`Path Parameters Example <https://github.com/Ramirisu/fitoria/blob/main/example/web/basic/path_parameter.cpp>`_

.. code-block:: cpp

   auto get_user(const request& req) -> awaitable<response>
   {
     co_return response::ok()
         .set_header(http::field::content_type, mime::text_plain())
         .set_body(fmt::format("user: {}", *req.path().get("user")));
   }
   
   int main()
   {
     auto ioc = net::io_context();
     auto server = http_server::builder(ioc)
                       .serve(route::get<"/api/v1/users/{user}">(get_user))
                       .build();
     server.bind("127.0.0.1", 8080);
   
     ioc.run();
   }
