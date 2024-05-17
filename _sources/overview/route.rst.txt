.. _route:

********************************************************************************
Route
********************************************************************************

fitoria supports **static path**, **parameterized path** and **wildcard matching**. The path is configured through the template parameter in order to perform *compile-time validation*.

.. code-block:: cpp

   route::get<"/api/v1/users/">(handler) // static

   route::get<"/api/v1/users/{user}">(handler) // parameterized

   route::get<"/api/v1/#wildcard">(handler) // wildcard


   route::get<"/api/v1/{">(handler) // error: static_assert failed: 'invalid path for route'

   route::get<"/api/v1/}">(handler) // error: static_assert failed: 'invalid path for route'

   route::get<"/api/v1/users/{user}x">(handler) // error: static_assert failed: 'invalid path for route'


Path Matching Priority
================================================================================

+---------------+----------+--------------------------+------------------------------------------------------------------------------------------------------------------------------------------------------------+
|     Type      | Priority |         Example          |                                                                           Format                                                                           |
+===============+==========+==========================+============================================================================================================================================================+
| Static        | 1        | ``/api/v1/users``        |                                                                                                                                                            |
+---------------+----------+--------------------------+------------------------------------------------------------------------------------------------------------------------------------------------------------+
| Parameterized | 2        | ``/api/v1/users/{user}`` | A name parameter enclosed within ``{}``. If a request path matches more than one parameterized routes, the one with longer static prefix will be returned. |
+---------------+----------+--------------------------+------------------------------------------------------------------------------------------------------------------------------------------------------------+
| Wildcard      | 3        | ``/api/v1/#any``         | A name parameter follow by ``#``. Note that wildcard must be the last segment of the path.                                                                 |
+---------------+----------+--------------------------+------------------------------------------------------------------------------------------------------------------------------------------------------------+
