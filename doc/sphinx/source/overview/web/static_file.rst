********************************************************************************
Static File
********************************************************************************

Use ``web::stream_file`` to serve static files. 

`Static Files Example <https://github.com/Ramirisu/fitoria/blob/main/example/web/basic/file.cpp>`_

.. note::
   
   The example code uses wildcard matching without checking the actual value of the path, which may cause serious security implications such as path traversal attack (``../``). Users should perform the validation on the input path carefully.

.. note::

   In order to use asynchronous file i/o on linux platform, ``liburing`` must be installed and define ``BOOST_ASIO_HAS_IO_URING`` to enable it. Note that asynchonous file i/o is not supported on MacOS.

.. code-block:: cpp

   auto get_static_file(const path_info& pi)
       -> awaitable<std::variant<stream_file, response>>
   {
     if (auto file = stream_file::open_readonly(co_await net::this_coro::executor,
                                                pi.at("file_path"));
         file) {
       co_return std::move(*file);
     }
   
     co_return response::not_found()
         .set_header(http::field::content_type, mime::text_plain())
         .set_body(fmt::format("requsted file was not found: \"{}\"",
                               pi.at("file_path")));
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
