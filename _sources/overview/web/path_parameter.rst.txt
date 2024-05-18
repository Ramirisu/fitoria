********************************************************************************
Path Parameter
********************************************************************************

Use ``request::path()`` to access the path parameters. (`Path Parameters Example <https://github.com/Ramirisu/fitoria/blob/main/example/web/path_parameter.cpp>`_)

.. code-block:: cpp

   namespace api::v1::users::get_user {
   auto api(const request& req) -> awaitable<response>
   {
     auto user = req.path().get("user");
     if (!user) {
       co_return response::bad_request().build();
     }
   
     co_return response::ok()
         .set_header(http::field::content_type,
                     http::fields::content_type::plaintext())
         .set_body(fmt::format("user: {}", user.value()));
   }
   }
   
   int main()
   {
     auto ioc = net::io_context();
     auto server = http_server::builder(ioc)
                       .serve(route::get<"/api/v1/users/{user}">(
                           api::v1::users::get_user::api))
                       .build();
     server.bind("127.0.0.1", 8080);
   
     ioc.run();
   }
