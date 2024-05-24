********************************************************************************
Unix Domain Socket
********************************************************************************

In addition to TCP connections, fitoria also supports connections with Unix domain socket (AF_UNIX). Users can call the ``http_server::bind_local(...)`` to bind specific file path. (`Unix Domain Socket Example <https://github.com/Ramirisu/fitoria/blob/main/example/web/unix_domain_socket.cpp>`_)

.. code-block:: cpp
   
   int main()
   {
     auto ioc = net::io_context();
     auto server
         = http_server::builder(ioc)
               .serve(
                   route::post<"/">([](std::string body) -> awaitable<response> {
                     co_return response::ok()
                         .set_header(http::field::content_type,
                                     http::fields::content_type::plaintext())
                         .set_body(body);
                   }))
               .build();
   
     const auto path = std::filesystem::temp_directory_path() / "fitoria";
     const auto tls_path = std::filesystem::temp_directory_path() / "fitoria.tls";
     std::filesystem::remove(path); // make sure no file exists
     std::filesystem::remove(tls_path); // make sure no file exists
   
     server.bind_local(path.string());
   
   #if defined(FITORIA_HAS_OPENSSL)
     auto ssl_ctx = cert::get_server_ssl_ctx(net::ssl::context::tls_server);
     server.bind_local(path.string(), ssl_ctx);
   #endif
   
     ioc.run();
   }

.. note::

   In order to enable TLS support, ``OpenSSL`` must be installed. 

.. seealso:: 

   Unix domain socket support for Windows starts from build 17063.
   
   https://devblogs.microsoft.com/commandline/af_unix-comes-to-windows/
