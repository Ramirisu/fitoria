.. _log_filtering:

********************************************************************************
Filtering
********************************************************************************

Use ``filter`` to configure the log filtering.

.. code-block:: cpp

   // result to `info`, `warning`, `error` and `fatal`.
   filter::at_least(level::info);
   
   // all levels
   filter::all();
   
   // load level config from the environment variable.
   // $ CPP_LOG=DEBUG ./my_server 
   filter::from_env();
