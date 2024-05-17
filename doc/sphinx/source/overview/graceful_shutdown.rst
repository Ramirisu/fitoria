.. _graceful_shutdown:

********************************************************************************
Graceful Shutdown
********************************************************************************

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
