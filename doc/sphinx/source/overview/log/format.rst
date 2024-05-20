********************************************************************************
Format Messages
********************************************************************************

``formatter`` allows users to customize the style of the messages being logged. Fields are mapped by named arguments and users can specify detailed format for each field.

.. code-block:: cpp

   auto writer = make_async_stdout_writer();
   writer->set_formatter(
       formatter::builder()
           // Custom format pattern
           .set_pattern("{TIME:%FT%TZ} {LV:} >> {MSG:} << {FUNC:}{FILE:}{LINE:}{COL:}")
           // Show full path of source file
           .set_file_name_style(file_name_style::full_path)
           // Show log level with colors
           .set_color_level_style());

+---------------+--------------------------------------------------------+--------------------------------------+
| Argument Name |                          Type                          |                Source                |
+===============+========================================================+======================================+
| ``{TIME:}``   | ``std::chrono::time_point<std::chrono::system_clock>`` | ``std::chrono::system_clock::now()`` |
+---------------+--------------------------------------------------------+--------------------------------------+
| ``{LV:}``     | ``log::level``                                         |                                      |
+---------------+--------------------------------------------------------+--------------------------------------+
| ``{MSG:}``    | ``std::string``                                        |                                      |
+---------------+--------------------------------------------------------+--------------------------------------+
| ``{LINE:}``   | ``std::uint32_t``                                      | ``std::source_location::current()``  |
+---------------+--------------------------------------------------------+--------------------------------------+
| ``{COL:}``    | ``std::uint32_t``                                      | ``std::source_location::current()``  |
+---------------+--------------------------------------------------------+--------------------------------------+
| ``{FILE:}``   | ``std::string_view``                                   | ``std::source_location::current()``  |
+---------------+--------------------------------------------------------+--------------------------------------+
| ``{FUNC:}``   | ``std::string_view``                                   | ``std::source_location::current()``  |
+---------------+--------------------------------------------------------+--------------------------------------+
