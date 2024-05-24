********************************************************************************
Extractor
********************************************************************************

Extractors provide a more convenient way to help users access information from the incoming ``request``. Users can specify as many extractors as compiler allows per handler.

Path
================================================================================

Use ``path_of<T>`` to extract path parameters into ``std::tuple<Ts...>`` or plain ``struct``.

`Path Extractor Example <https://github.com/Ramirisu/fitoria/blob/main/example/web/extractor/path_of.cpp>`_

.. code-block:: cpp

   auto get_order_tuple(path_of<std::tuple<std::string, std::uint64_t>> order)
       -> awaitable<response>
   {
     auto [user, order_id] = order;
   
     co_return response::ok()
         .set_header(http::field::content_type,
                     http::fields::content_type::plaintext())
         .set_body(fmt::format("user: {}, order_id: {}", user, order_id));
   }
   
   struct order_t {
     std::string user;
     std::uint64_t order_id;
   };
   
   auto get_order_struct(path_of<order_t> order) -> awaitable<response>
   {
     co_return response::ok()
         .set_header(http::field::content_type,
                     http::fields::content_type::plaintext())
         .set_body(
             fmt::format("user: {}, order_id: {}", order.user, order.order_id));
   }
   
   int main()
   {
     auto ioc = net::io_context();
     auto server = http_server::builder(ioc)
                       .serve(route::get<"/tuple/users/{user}/orders/{order_id}">(
                           get_order_tuple))
                       .serve(route::get<"/struct/users/{user}/orders/{order_id}">(
                           get_order_struct))
                       .build();
   
     server.bind("127.0.0.1", 8080);
   
     ioc.run();
   }

Query
================================================================================

Use ``query_of<T>`` to extract query string parameters into plain ``struct``. 

`Query Extractor Example <https://github.com/Ramirisu/fitoria/blob/main/example/web/extractor/query_of.cpp>`_

.. code-block:: cpp

   struct order_t {
     std::string user;
     std::uint64_t order_id;
   };
   
   auto get_order(query_of<order_t> order) -> awaitable<response>
   {
     co_return response::ok()
         .set_header(http::field::content_type,
                     http::fields::content_type::plaintext())
         .set_body(
             fmt::format("user: {}, order_id: {}", order.user, order.order_id));
   }

Form
================================================================================

Use ``form_of<T>`` to extract urlencoded form from body into plain ``struct``.

`Form Extractor Example <https://github.com/Ramirisu/fitoria/blob/main/example/web/extractor/form_of.cpp>`_

.. code-block:: cpp

   namespace api::v1::login {
   
   struct user_t {
     std::string username;
     std::string password;
   };
   
   auto api(form_of<user_t> user) -> awaitable<response>
   {
     if (user.username != "fitoria" || user.password != "123456") {
       co_return response::unauthorized().build();
     }
   
     co_return response::ok().build();
   }
   }

State
================================================================================

Use ``state_of<T>`` to extract shared states. Note that unlike ``request::state<T>()`` which returns ``optional<T&>``, ``state_of`` extractor *copy the value*. 

`State Extractor Example <https://github.com/Ramirisu/fitoria/blob/main/example/web/extractor/state_of.cpp>`_

.. code-block:: cpp

   using counter_t = std::atomic<std::size_t>;
   
   auto get_index(state_of<std::shared_ptr<counter_t>> counter)
       -> awaitable<response>
   {
     co_return response::ok()
         .set_header(http::field::content_type,
                     http::fields::content_type::plaintext())
         .set_body(fmt::format("index page has been acquired {} times.",
                               counter->fetch_add(1, std::memory_order_relaxed)));
   }

.. note:: 

   To enable extractor for custom type, implement customization point ``from_request_t``.


Built-in Extractors
================================================================================

+-----------------------+-----------------------------------------------------------+----------------+------------------------------------------------------------------------------------------------------+
|       Extractor       |                        Description                        | Body Extractor |                                                                                                      |
+=======================+===========================================================+================+======================================================================================================+
| ``web::request``      | Extract whole ``request``.                                | no             |                                                                                                      |
+-----------------------+-----------------------------------------------------------+----------------+------------------------------------------------------------------------------------------------------+
| ``http::version``     | Extract HTTP version.                                     | no             |                                                                                                      |
+-----------------------+-----------------------------------------------------------+----------------+------------------------------------------------------------------------------------------------------+
| ``http::header``      | Extract headers from request headers.                     | no             |                                                                                                      |
+-----------------------+-----------------------------------------------------------+----------------+------------------------------------------------------------------------------------------------------+
| ``web::connect_info`` | Extract connection info.                                  | no             |                                                                                                      |
+-----------------------+-----------------------------------------------------------+----------------+------------------------------------------------------------------------------------------------------+
| ``web::path_info``    | Extract path info parameter.                              | no             |                                                                                                      |
+-----------------------+-----------------------------------------------------------+----------------+------------------------------------------------------------------------------------------------------+
| ``web::path_of<T>``   | Extract path parameter into type ``T``.                   | no             | * ``T = std::tuple<Ts...>``, parameters are extracted in the order where they are in the path.       |
|                       |                                                           |                | * ``T = aggregate``, parameters are extracted to the field of their name.                            |
+-----------------------+-----------------------------------------------------------+----------------+------------------------------------------------------------------------------------------------------+
| ``web::query_map``    | Extract query string parameters.                          | no             |                                                                                                      |
+-----------------------+-----------------------------------------------------------+----------------+------------------------------------------------------------------------------------------------------+
| ``web::query_of<T>``  | Extract query string parameters into type ``T``           | no             | ``T = aggregate``, parameters are extracted to the field of their name.                              |
+-----------------------+-----------------------------------------------------------+----------------+------------------------------------------------------------------------------------------------------+
| ``web::state_of<T>``  | Extract shared state of type ``T``.                       | no             | Note that unlike ``request::state<T>()`` which returns ``optional<T&>``, extractor *copy the value*. |
+-----------------------+-----------------------------------------------------------+----------------+------------------------------------------------------------------------------------------------------+
| ``web::websocket``    | Extract as websocket.                                     | no             |                                                                                                      |
+-----------------------+-----------------------------------------------------------+----------------+------------------------------------------------------------------------------------------------------+
| ``web::form_of<T>``   | Extract urlencoded form from body into type ``T``         | yes            | ``T = aggregate``, parameters are extracted to the field of their name.                              |
+-----------------------+-----------------------------------------------------------+----------------+------------------------------------------------------------------------------------------------------+
| ``web::json_of<T>``   | Extract body and parse it into json and convert to ``T``. | yes            |                                                                                                      |
+-----------------------+-----------------------------------------------------------+----------------+------------------------------------------------------------------------------------------------------+
| ``std::string``       | Extract body as ``std::string``.                          | yes            |                                                                                                      |
+-----------------------+-----------------------------------------------------------+----------------+------------------------------------------------------------------------------------------------------+
| ``std::vector<T>``    | Extract body as ``std::vector<T>``.                       | yes            |                                                                                                      |
+-----------------------+-----------------------------------------------------------+----------------+------------------------------------------------------------------------------------------------------+

.. note:: 

   The **body extractor** can only be used at most once in the request handlers since it consumes the body.

