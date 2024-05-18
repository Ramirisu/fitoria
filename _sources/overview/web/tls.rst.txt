********************************************************************************
TLS
********************************************************************************

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
