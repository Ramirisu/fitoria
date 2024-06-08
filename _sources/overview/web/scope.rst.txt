********************************************************************************
Scope
********************************************************************************

Use ``scope`` to group one or more ``route`` s. 

`Scope Example <https://github.com/Ramirisu/fitoria/blob/main/example/web/basic/scope.cpp>`_

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
