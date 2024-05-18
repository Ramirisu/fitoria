********************************************************************************
Maximize Performance
********************************************************************************

fitoria is built on top of ``boost::asio`` (aliasing as ``fitoria::net`` namespace) and the ``executory_type`` defaults to ``net::any_io_executor`` which is a polymorphic executor and may hurt the performance. 

Define ``FITORIA_USE_IO_CONTEXT_EXECUTOR`` to use ``net::io_context::executor_type`` as the default executor type.

.. code-block:: cpp

   #define FITORIA_USE_IO_CONTEXT_EXECUTOR
   #include <fitoria/web.hpp>


Or define ``FITORIA_USE_CUSTOM_EXECUTOR`` to use custom executor.

.. code-block:: cpp

   #define FITORIA_USE_CUSTOM_EXECUTOR my_executor
   #include <fitoria/web.hpp>
