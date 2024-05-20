********************************************************************************
Integration
********************************************************************************

``fitoria`` integrates CMake as the build system and can be easily built on many platforms.


CMake
================================================================================

Clone the source code into your project and add following code in the ``CMakeLists.txt`` file

.. code-block:: cmake
   
   add_subdirectory(fitoria)

   find_package(fitoria)
   target_link_libraries(<target> PRIVATE fitoria)

.. note:: 

   ``vcpkg`` integration will be supported in the future.
