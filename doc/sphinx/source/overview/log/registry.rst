********************************************************************************
Registry
********************************************************************************

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
