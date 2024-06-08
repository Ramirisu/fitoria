********************************************************************************
Response
********************************************************************************

Response converters help users to automatically convert returned type into ``response``. 

`Response Example <https://github.com/Ramirisu/fitoria/blob/main/example/web/basic/response.cpp>`_

.. code-block:: cpp

   auto resp() -> awaitable<response>
   {
     co_return response::ok()
         .set_header(http::field::content_type, mime::text_plain())
         .set_body("Hello World!");
   }
   
   auto text() -> awaitable<std::string>
   {
     co_return "Hello World!";
   }
   
   auto binary() -> awaitable<std::vector<std::uint8_t>>
   {
     co_return std::vector<std::uint8_t> { 0x48, 0x65, 0x6c, 0x6c, 0x6f, 0x20,
                                           0x57, 0x6f, 0x72, 0x6c, 0x64, 0x21 };
   }
   
   auto variant(path_of<std::tuple<bool>> path)
       -> awaitable<std::variant<std::string, std::vector<std::uint8_t>>>
   {
     auto [text] = path;
     if (text) {
       co_return "Hello World!";
     } else {
       co_return std::vector<std::uint8_t> { 0x48, 0x65, 0x6c, 0x6c, 0x6f, 0x20,
                                             0x57, 0x6f, 0x72, 0x6c, 0x64, 0x21 };
     }
   }

.. note:: 

   To enable response converter for custom type, implement customization point ``to_response_t``.

.. code-block:: cpp

   class my_error {
     std::string msg_;
   
   public:
     my_error(std::string msg)
         : msg_(std::move(msg))
     {
     }
   
     auto message() const noexcept -> const std::string&
     {
       return msg_;
     }
   
     template <decay_to<my_error> Self>
     friend auto tag_invoke(fitoria::web::to_response_t, Self&& self) -> response
     {
       return response::not_found()
           .set_header(http::field::content_type, mime::text_plain())
           .set_body(self.message());
     }
   };
   
   auto err() -> awaitable<my_error>
   {
     co_return my_error("You will never get anything!");
   }


Built-in Response Converters
================================================================================

+-------------------------+--------------------------------------------------+--------------------------------------------------------------------------------------------------------------+
|   Response Converter    |                   Description                    |                                                                                                              |
+=========================+==================================================+==============================================================================================================+
| ``std::string``         | Put string into ``response`` body.               | ``Content-Type: text/plain`` will be inserted.                                                               |
+-------------------------+--------------------------------------------------+--------------------------------------------------------------------------------------------------------------+
| ``std::vector<T>``      | Put ``std::vector<T>`` into ``response`` body.   | ``Content-Type: application/octet-stream`` will be inserted.                                                 |
+-------------------------+--------------------------------------------------+--------------------------------------------------------------------------------------------------------------+
| ``std::variant<Ts...>`` | Convert any of ``Ts...`` into ``response`` body. | Note that ``Ts...`` must be convertible to ``response``.                                                     |
+-------------------------+--------------------------------------------------+--------------------------------------------------------------------------------------------------------------+
| ``expected<T, E>``      | Convert ``T`` or ``E`` into ``response`` body.   | Note that ``T`` and ``E`` must be convertible to ``response``.                                               |
+-------------------------+--------------------------------------------------+--------------------------------------------------------------------------------------------------------------+
| ``stream_file``         | Put ``stream_file`` into ``response`` body.      | ``Content-Type: application/octet-stream`` will be inserted and ``Transfer-Encoding: Chunked`` will be used. |
+-------------------------+--------------------------------------------------+--------------------------------------------------------------------------------------------------------------+

