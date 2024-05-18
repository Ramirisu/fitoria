********************************************************************************
Log Messages
********************************************************************************

Use ``log(level, fmt, ...)``, ``trace(fmt, ...)``, ``debug(fmt, ...)``, ``info(fmt, ...)``, ``warning(fmt, ...)``, ``error(fmt, ...)``, ``fatal(fmt, ...)`` to write logs to the default logger.

.. code-block:: cpp

  log(level::info, "price: {}", 100);
  
  trace("price: {}", 100);
  debug("price: {}", 100);
  info("price: {}", 100);
  warning("price: {}", 100);
  error("price: {}", 100);
  fatal("price: {}", 100);
