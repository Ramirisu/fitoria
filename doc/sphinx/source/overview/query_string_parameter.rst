.. _query_string_parameter:

********************************************************************************
Query String Parameter
********************************************************************************

Use ``request::query()`` to access the query string parameters. (`Query String Parameters Example <https://github.com/Ramirisu/fitoria/blob/main/example/web/query_string.cpp>`_)

.. code-block:: cpp

   auto get_user(const request& req) -> awaitable<response>
   {
     auto user = req.query().get("user");
     if (!user) {
       co_return response::bad_request().build();
     }
   
     co_return response::ok()
         .set_header(http::field::content_type,
                     http::fields::content_type::plaintext())
         .set_body(fmt::format("user: {}", user.value()));
   }
