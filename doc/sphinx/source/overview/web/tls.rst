********************************************************************************
TLS
********************************************************************************

Plain TCP connections are insecure, fitoria supports TLS connections using ``OpenSSL``. Users can call ``http_server::bind(..., ssl_ctx)`` with ``net::ssl::context`` to enable the TLS connections.

`TLS Example <https://github.com/Ramirisu/fitoria/blob/main/example/web/basic/tls.cpp>`_

.. code-block:: cpp
   
   int main()
   {
     auto ioc = net::io_context();
     auto server
         = http_server::builder(ioc)
               .serve(route::get<"/">([]() -> awaitable<response> {
                 co_return response::ok()
                     .set_header(http::field::content_type, mime::text_plain())
                     .set_body("Hello World!");
               }))
               .build();
   
     server.bind("127.0.0.1", 8080);
     
     auto ssl_ctx = cert::get_server_ssl_ctx(net::ssl::context::tls_server);
     server.bind("127.0.0.1", 8443, ssl_ctx);
   
     ioc.run();
   }

.. note::

   In order to enable TLS support, ``OpenSSL`` must be installed. 
