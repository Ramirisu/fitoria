.. fitoria documentation master file, created by
   sphinx-quickstart on Tue May 14 19:02:49 2024.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

Welcome to fitoria's documentation!
===================================

.. toctree::
   :maxdepth: 2
   :caption: Contents:


Table of Contents
=================

* :ref:`web`

  * :ref:`getting_started`
  * :ref:`http_server`
  
    * :ref:`tls`
    * :ref:`threading`
    * :ref:`graceful_shutdown`
    * :ref:`maximize_performance`

  * :ref:`handlers`

    * :ref:`method`
    * :ref:`route`
    * :ref:`scope`
    * :ref:`path_parameters`
    * :ref:`query_string_parameters`
    * :ref:`state`
    * :ref:`extractor`

      * :ref:`path_extractor`
      * :ref:`query_extractor`
      * :ref:`form_extractor`

    * :ref:`middleware`
    * :ref:`static_files`
    * :ref:`websocket`
    * :ref:`unit_testing`

* :ref:`log`

  * :ref:`log_level`
  
    * :ref:`log_level_filtering`

  * :ref:`logger`

    * :ref:`logger_registry`
    * :ref:`log_messages`

.. _web:


Web
===

Namespace ``fitoria::web`` provides web utilities.

.. _getting_started:

Getting Started
---------------

The following example demonstrates how to create an simple ``http_server`` and attach handlers to it. (`Getting Started Example <https://github.com/Ramirisu/fitoria/blob/main/example/web/getting_started.cpp>`_)

.. code-block:: cpp

   #include <fitoria/web.hpp>
   
   using namespace fitoria;
   using namespace fitoria::web;
   
   auto hello_world() -> awaitable<response>
   {
     co_return response::ok()
         .set_header(http::field::content_type,
                     http::fields::content_type::plaintext())
         .set_body("Hello World!");
   }
   
   auto echo(std::string body) -> awaitable<response>
   {
     co_return response::ok()
         .set_header(http::field::content_type,
                     http::fields::content_type::plaintext())
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


.. _http_server:

HTTP Server
-----------


.. _tls:

TLS
^^^

First we have to create a ``net::ssl::context`` with server certificates and desired TLS configurations. And then call ``http_server::bind_ssl(...)`` with the ssl context to enable TLS for incoming connections. (`TLS Example <https://github.com/Ramirisu/fitoria/blob/main/example/web/tls.cpp>`_)

.. note::

   In order to enable TLS support, ``openssl`` must be installed. 

.. code-block:: cpp
   
   int main()
   {
     auto ioc = net::io_context();
     auto server = http_server::builder(ioc)
                       .serve(route::get<"/">([]() -> awaitable<response> {
                         co_return response::ok()
                             .set_header(http::field::content_type,
                                         http::fields::content_type::plaintext())
                             .set_body("Hello World!");
                       }))
                       .build();
   
     server.bind("127.0.0.1", 8080);
     
     auto ssl_ctx = cert::get_server_ssl_ctx(net::ssl::context::tls_server);
     server.bind_ssl("127.0.0.1", 8443, ssl_ctx);
   
     ioc.run();
   }


.. _threading:

Threading
^^^^^^^^^

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


.. _graceful_shutdown:

Graceful Shutdown
^^^^^^^^^^^^^^^^^

Use ``net::signal_set`` to handle signals and shutdown the server gracefully. (`Graceful Shutdown Example <https://github.com/Ramirisu/fitoria/blob/main/example/web/graceful_shutdown.cpp>`_)

.. code-block:: cpp

   int main()
   {
     auto ioc = net::io_context();
     auto server = http_server::builder(ioc).build();
     server.bind("127.0.0.1", 8080);
   
     net::signal_set signal(ioc, SIGINT, SIGTERM);
     signal.async_wait([&](auto, auto) { ioc.stop(); });
   
     ioc.run();
   }


.. _maximize_performance:

Maximize Performance
^^^^^^^^^^^^^^^^^^^^

fitoria is built on top of ``boost::asio`` (aliasing as ``fitoria::net`` namespace) and the ``executory_type`` defaults to ``net::any_io_executor`` which is a polymorphic executor and may hurt the performance. 

Define ``FITORIA_USE_IO_CONTEXT_EXECUTOR`` to use ``net::io_context::executor_type`` as the default executor type.

.. code-block:: cpp

   #define FITORIA_USE_IO_CONTEXT_EXECUTOR
   #include <fitoria/web.hpp>


Or define ``FITORIA_USE_CUSTOM_EXECUTOR`` to use custom executor.

.. code-block:: cpp

   #define FITORIA_USE_CUSTOM_EXECUTOR my_executor
   #include <fitoria/web.hpp>


.. _handlers:

Handlers
--------


.. _method:

Method
^^^^^^

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


.. _route:

Route
^^^^^

fitoria supports **static path**, **parameterized path** and **wildcard matching**. The path is configured through the template parameter in order to perform *compile-time validation*.

.. code-block:: cpp

   route::get<"/api/v1/users/">(handler) // static

   route::get<"/api/v1/users/{user}">(handler) // parameterized

   route::get<"/api/v1/#wildcard">(handler) // wildcard


   route::get<"/api/v1/{">(handler) // error: static_assert failed: 'invalid path for route'

   route::get<"/api/v1/}">(handler) // error: static_assert failed: 'invalid path for route'

   route::get<"/api/v1/users/{user}x">(handler) // error: static_assert failed: 'invalid path for route'


Path Matching Priority
""""""""""""""""""""""

+---------------+----------+--------------------------+------------------------------------------------------------------------------------------------------------------------------------------------------------+
|     Type      | Priority |         Example          |                                                                           Format                                                                           |
+===============+==========+==========================+============================================================================================================================================================+
| Static        | 1        | ``/api/v1/users``        |                                                                                                                                                            |
+---------------+----------+--------------------------+------------------------------------------------------------------------------------------------------------------------------------------------------------+
| Parameterized | 2        | ``/api/v1/users/{user}`` | A name parameter enclosed within ``{}``. If a request path matches more than one parameterized routes, the one with longer static prefix will be returned. |
+---------------+----------+--------------------------+------------------------------------------------------------------------------------------------------------------------------------------------------------+
| Wildcard      | 3        | ``/api/v1/#any``         | A name parameter follow by ``#``. Note that wildcard must be the last segment of the path.                                                                 |
+---------------+----------+--------------------------+------------------------------------------------------------------------------------------------------------------------------------------------------------+


.. _scope:

Scope
^^^^^

Use ``scope`` to group one or more ``route`` s. (`Scope Example <https://github.com/Ramirisu/fitoria/blob/main/example/web/scope.cpp>`_)

.. code-block:: cpp

   int main()
   {
     auto ioc = net::io_context();
     auto server
         = http_server::builder(ioc)
               .serve(
                   scope()
                       .use(middleware::logger())
                       .serve(scope<"/api/v1">()
                                  .serve(route::post<"/register">(api::v1::reg))
                                  .serve(route::post<"/login">(api::v1::login)))
                       .serve(scope<"/api/v2">()
                                  .serve(route::post<"/register">(api::v2::reg))
                                  .serve(route::post<"/login">(api::v2::login))))
               .build();
     server.bind("127.0.0.1", 8080);
   
     ioc.run();
   }


.. _path_parameters:

Path Parameters
^^^^^^^^^^^^^^^

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


.. _query_string_parameters:

Query String Parameters
^^^^^^^^^^^^^^^^^^^^^^^

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


.. _state:

State
^^^^^

fitoria provides a way to share data between multiple ``route`` s under the same ``scope`` by configuring the data via ``scope::use_state(State&&)``. (`State Example <https://github.com/Ramirisu/fitoria/blob/main/example/web/state.cpp>`_)

.. code-block:: cpp

   namespace cache {
   class simple_cache {
     using map_type = unordered_string_map<std::string>;
   
   public:
     optional<map_type::mapped_type> get(std::string_view key) const
     {
       auto lock = std::shared_lock { mutex_ };
       if (auto it = map_.find(key); it != map_.end()) {
         return it->second;
       }
   
       return nullopt;
     }
   
     bool put(const std::string& key, std::string value)
     {
       auto lock = std::unique_lock { mutex_ };
       return map_.insert_or_assign(key, std::move(value)).second;
     }
   
   private:
     map_type map_;
     mutable std::shared_mutex mutex_;
   };
   
   using simple_cache_ptr = std::shared_ptr<simple_cache>;
   
   auto put(const request& req) -> awaitable<response>
   {
     auto key = req.path().get("key");
     auto value = req.path().get("value");
     if (!key || !value) {
       co_return response::bad_request().build();
     }
   
     auto cache = req.state<simple_cache_ptr>();
     if (!cache) {
       co_return response::internal_server_error().build();
     }
   
     if ((*cache)->put(*key, *value)) {
       co_return response::created().build();
     } else {
       co_return response::accepted().build();
     }
   }
   
   auto get(const request& req) -> awaitable<response>
   {
     auto key = req.path().get("key");
     if (!key) {
       co_return response::bad_request().build();
     }
   
     auto cache = req.state<simple_cache_ptr>();
     if (!cache) {
       co_return response::internal_server_error().build();
     }
   
     if (auto value = (*cache)->get(*key); value) {
       co_return response::ok()
           .set_header(http::field::content_type,
                       http::fields::content_type::plaintext())
           .set_body(*value);
     } else {
       co_return response::not_found().build();
     }
   }
   
   }
   
   int main()
   {
     auto cache = std::make_shared<cache::simple_cache_ptr>();
   
     auto ioc = net::io_context();
     auto server = http_server::builder(ioc)
                       .serve(scope<"/cache">()
                                  .use_state(cache)
                                  .serve(route::put<"/{key}/{value}">(cache::put))
                                  .serve(route::get<"/{key}">(cache::get)))
                       .build();
     server.bind("127.0.0.1", 8080);
   
     ioc.run();
   }


.. _extractor:

Extractor
^^^^^^^^^

Extractors provide a more convenient way to help user access information from ``request``. Users can specify as many extractors as compiler allows per handler.

.. _path_extractor:

Path
""""

Use ``path_of<T>`` to extract path parameters into ``std::tuple<Ts...>`` or plain ``struct``. (`Path Extractor Example <https://github.com/Ramirisu/fitoria/blob/main/example/web/extractor/path_of.cpp>`_)

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

.. _query_extractor:

Query
"""""

Use ``query_of<T>`` to extract query string parameters into plain ``struct``. (`Query Extractor Example <https://github.com/Ramirisu/fitoria/blob/main/example/web/extractor/query_of.cpp>`_)

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

.. _form_extractor:

Form
""""

Use ``form_of<T>`` to extract urlencoded form from body into plain ``struct``. (`Form Extractor Example <https://github.com/Ramirisu/fitoria/blob/main/example/web/extractor/form_of.cpp>`_)

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

.. note:: 

   To enable extractor for custom type, implement customization point ``from_request_t``.


Built-in Extractors
"""""""""""""""""""

+-----------------------+-----------------------------------------------------------+----------------+---------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
|       Extractor       |                        Description                        | Body Extractor |                                                                                                                                                                           |
+=======================+===========================================================+================+===========================================================================================================================================================================+
| ``web::request``      | Extract whole ``request``.                                | no             |                                                                                                                                                                           |
+-----------------------+-----------------------------------------------------------+----------------+---------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| ``http::version``     | Extract http version.                                     | no             |                                                                                                                                                                           |
+-----------------------+-----------------------------------------------------------+----------------+---------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| ``http::header``      | Extract headers from request headers.                     | no             |                                                                                                                                                                           |
+-----------------------+-----------------------------------------------------------+----------------+---------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| ``web::connect_info`` | Extract connection info.                                  | no             |                                                                                                                                                                           |
+-----------------------+-----------------------------------------------------------+----------------+---------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| ``web::path_info``    | Extract path info parameter.                              | no             |                                                                                                                                                                           |
+-----------------------+-----------------------------------------------------------+----------------+---------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| ``web::path_of<T>``   | Extract path parameter into type ``T``.                   | no             | ``T = std::tuple<Ts...>``, parameters are extracted in the order where they are in the path.<br/> ``T = aggregate``, parameters are extracted to the field of their name. |
+-----------------------+-----------------------------------------------------------+----------------+---------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| ``web::query_map``    | Extract query string parameters.                          | no             |                                                                                                                                                                           |
+-----------------------+-----------------------------------------------------------+----------------+---------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| ``web::query_of<T>``  | Extract query string parameters into type ``T``           | no             | ``T = aggregate``, parameters are extracted to the field of their name.                                                                                                   |
+-----------------------+-----------------------------------------------------------+----------------+---------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| ``web::state_of<T>``  | Extract shared state of type ``T``.                       | no             | Note that unlike ``request::state<T>()`` which returns ``optional<T&>``, extractor *copy the value*.                                                                      |
+-----------------------+-----------------------------------------------------------+----------------+---------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| ``web::websocket``    | Extract as websocket.                                     | no             |                                                                                                                                                                           |
+-----------------------+-----------------------------------------------------------+----------------+---------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| ``web::form_of<T>``   | Extract urlencoded form from body into type ``T``         | yes            | ``T = aggregate``, parameters are extracted to the field of their name.                                                                                                   |
+-----------------------+-----------------------------------------------------------+----------------+---------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| ``web::json_of<T>``   | Extract body and parse it into json and convert to ``T``. | yes            |                                                                                                                                                                           |
+-----------------------+-----------------------------------------------------------+----------------+---------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| ``std::string``       | Extract body as ``std::string``.                          | yes            |                                                                                                                                                                           |
+-----------------------+-----------------------------------------------------------+----------------+---------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| ``std::vector<T>``    | Extract body as ``std::vector<T>``.                       | yes            |                                                                                                                                                                           |
+-----------------------+-----------------------------------------------------------+----------------+---------------------------------------------------------------------------------------------------------------------------------------------------------------------------+

.. note:: 

   The **body extractor** can only be used at most once in the request handlers since it consumes the body.


.. _middleware:

Middleware
^^^^^^^^^^

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


.. _static_files:

Static Files
^^^^^^^^^^^^

Use ``web::stream_file`` to serve static files. (`Static Files Example <https://github.com/Ramirisu/fitoria/blob/main/example/web/file.cpp>`_)

.. note::
   
   The example code uses wildcard matching without checking the actual value of the path, which may cause serious security implications such as path traversal attack (``../``). Users should perform the validation on the input path carefully.

.. note::

   In order to use asynchronous file i/o on linux platform, ``liburing`` must be installed and define ``BOOST_ASIO_HAS_IO_URING`` to enable it. Note that asynchonous file i/o is not supported on MacOS.

.. code-block:: cpp

   auto get_static_file(const path_info& pi)
       -> awaitable<std::variant<stream_file, response>>
   {
     auto path = pi.at("file_path");
     if (auto file = co_await stream_file::async_open_readonly(path); file) {
       co_return std::move(*file);
     }
   
     co_return response::not_found()
         .set_header(http::field::content_type,
                     http::fields::content_type::plaintext())
         .set_body(fmt::format("requsted file was not found: \"{}\"", path));
   }
   
   int main()
   {
     auto ioc = net::io_context();
     auto server = http_server::builder(ioc)
                       .serve(route::get<"/static/#file_path">(get_static_file))
                       .build();
     server.bind("127.0.0.1", 8080);
   
     ioc.run();
   }


.. _websocket:

WebSocket
^^^^^^^^^

fitoria supports websocket by using the ``websocket`` extractor. (`WebSocket Example <https://github.com/Ramirisu/fitoria/blob/main/example/web/websocket.cpp>`_)

.. code-block:: cpp

   auto ws_handler(websocket::context& ctx) -> awaitable<void>
   {
     for (auto msg = co_await ctx.async_read(); msg;
          msg = co_await ctx.async_read()) {
       if (auto binary = std::get_if<websocket::binary_t>(&*msg); binary) {
         co_await ctx.async_write_binary(
             std::span { binary->value.data(), binary->value.size() });
       } else if (auto text = std::get_if<websocket::text_t>(&*msg); text) {
         co_await ctx.async_write_text(text->value);
       } else if (auto c = std::get_if<websocket::close_t>(&*msg); c) {
         break;
       }
     }
   }
   
   auto http_handler(websocket ws) -> awaitable<response>
   {
     ws.set_handshake_timeout(std::chrono::seconds(5));
     ws.set_idle_timeout(std::chrono::seconds(30));
     ws.set_keep_alive_pings(true);
     co_return ws.set_handler(ws_handler);
   }
   
   int main()
   {
     auto ioc = net::io_context();
     auto server = http_server::builder(ioc)
                       .serve(route::get<"/ws">(http_handler))
                       .build();
     server.bind("127.0.0.1", 8080);
   
     ioc.run();
   }


.. _unit_testing:

Unit Testing
^^^^^^^^^^^^

Unit testing is important for software quality assurance, fitoria provides ``http_server::serve_request()`` to test the mock ``test_request`` for the handlers without creating any TCP connections. (`Testing Example <https://github.com/Ramirisu/fitoria/blob/main/example/web/unittesting.cpp>`_)


.. _log:


Log
===

Namespace ``fitoria::log`` provides log utilities.

.. _log_level:

Log Level
---------

There are six log levels ``level::trace``, ``level::debug``, ``level::info``, ``level::warning``, ``level::error`` and ``level::fatal``.

.. code-block:: cpp

   enum class level {
     trace,
     debug,
     info,
     warning,
     error,
     fatal,
   };


.. _log_level_filtering:

Log Level Filtering
^^^^^^^^^^^^^^^^^^^

Use ``filter`` to configure the log level filtering.

.. code-block:: cpp

   // result to `info`, `warning`, `error` and `fatal`.
   filter::at_least(level::info);
   
   // all levels
   filter::all();
   
   // load level config from the environment variable.
   // $ CPP_LOG=DEBUG ./my_server 
   filter::from_env();


.. _logger:

Logger
------

.. _logger_registry:

Registry
^^^^^^^^

``registry::global()`` can be used to access the singleton which allows users to register the logger globally, then we can create the logger by ``async_logger::builder``.

.. code-block:: cpp

   registry::global().set_default_logger(
         async_logger::builder()
             .set_filter(filter::at_least(level::trace))
             .build());

After registering the logger, one or more ``async_writer`` s should be attached to the logger in order to determine where/how to log the messages.

.. code-block:: cpp

   // an stdout writer
   registry::global().default_logger()->add_writer(make_async_stdout_writer());
   
   
   // a file writer
   registry::global().default_logger()->add_writer(
       make_async_stream_file_writer("./my_server.log"));


.. _log_messages:

Log Messages
^^^^^^^^^^^^

Use ``log(level, fmt, ...)``, ``trace(fmt, ...)``, ``debug(fmt, ...)``, ``info(fmt, ...)``, ``warning(fmt, ...)``, ``error(fmt, ...)``, ``fatal(fmt, ...)`` to write logs to the default logger.

.. code-block:: cpp

  log(level::info, "price: {}", 100);
  
  trace("price: {}", 100);
  debug("price: {}", 100);
  info("price: {}", 100);
  warning("price: {}", 100);
  error("price: {}", 100);
  fatal("price: {}", 100);


Format Messages
^^^^^^^^^^^^^^^

``formatter`` allows user to customize the style of the messages being logged. Fields are mapped by named arguments and users can specify detailed format for each field.

.. code-block:: cpp

   auto writer = make_async_stdout_writer();
   writer->set_formatter(
       formatter::builder()
           // Custom format pattern
           .set_pattern("{TIME:%FT%TZ} {LV:} >> {MSG:} << {FUNC:}{FILE:}{LINE:}{COL:}")
           // Show full path of source file
           .set_file_name_style(file_name_style::full_path)
           // Show log level with colors
           .set_color_level_style());

+---------------+--------------------------------------------------------+--------------------------------------+
| Argument Name |                          Type                          |                Source                |
+===============+========================================================+======================================+
| ``{TIME:}``   | ``std::chrono::time_point<std::chrono::system_clock>`` | ``std::chrono::system_clock::now()`` |
+---------------+--------------------------------------------------------+--------------------------------------+
| ``{LV:}``     | ``log::level``                                         |                                      |
+---------------+--------------------------------------------------------+--------------------------------------+
| ``{MSG:}``    | ``std::string``                                        |                                      |
+---------------+--------------------------------------------------------+--------------------------------------+
| ``{LINE:}``   | ``std::uint32_t``                                      | ``std::source_location::current()``  |
+---------------+--------------------------------------------------------+--------------------------------------+
| ``{COL:}``    | ``std::uint32_t``                                      | ``std::source_location::current()``  |
+---------------+--------------------------------------------------------+--------------------------------------+
| ``{FILE:}``   | ``std::string_view``                                   | ``std::source_location::current()``  |
+---------------+--------------------------------------------------------+--------------------------------------+
| ``{FUNC:}``   | ``std::string_view``                                   | ``std::source_location::current()``  |
+---------------+--------------------------------------------------------+--------------------------------------+

.. _license:


License
=======

This project is distributed under the `Boost Software License 1.0 <https://www.boost.org/LICENSE_1_0.txt>`_
