********************************************************************************
Static File
********************************************************************************

Use ``web::static_file`` to serve static files. ``Range`` and ``If-None-Match`` from the ``request`` will be handled automatically and ``Content-Type`` and ``Content-Disposition`` for the ``response`` will be obtained from the provided file extension.

`Static Files Example <https://github.com/Ramirisu/fitoria/blob/main/example/web/basic/static_file.cpp>`_

.. note::
   
   The example code uses wildcard matching without checking the actual value of the path, which may cause serious security implications such as path traversal attack (``../``). Users should perform the validation on the input path carefully.

.. note::

   In order to use asynchronous file i/o on linux platform, ``liburing`` must be installed and define ``BOOST_ASIO_HAS_IO_URING`` to enable it. Note that asynchonous file i/o is not supported on MacOS.

.. code-block:: cpp

   auto serve_file(path_of<std::tuple<std::string>> path, const request& req)
       -> awaitable<expected<static_file, response>>
   {
     auto [file_path] = path;
   
     co_return static_file::open(co_await net::this_coro::executor, file_path, req)
         .transform_error([&file_path](auto) -> response {
           return response::not_found()
               .set_header(http::field::content_type, mime::text_plain())
               .set_body(
                   fmt::format("requsted file was not found: \"{}\"", file_path));
         });
   }
   
   int main()
   {
     auto ioc = net::io_context();
     auto server = http_server::builder(ioc)
                       .serve(route::get<"/#file_path">(serve_file))
                       .build();
     server.bind("127.0.0.1", 8080);
   
     ioc.run();
   }
