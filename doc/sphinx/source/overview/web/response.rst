********************************************************************************
Response
********************************************************************************

Response converters help user to automatically convert returned type into ``response``. (`Response Example <https://github.com/Ramirisu/fitoria/blob/main/example/web/response.cpp>`_)

.. code-block:: cpp

   auto resp() -> awaitable<response>
   {
     co_return response::ok()
         .set_header(http::field::content_type,
                     http::fields::content_type::plaintext())
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
   
   auto variant(path_of<std::tuple<std::string>> path)
       -> awaitable<std::variant<std::string, std::vector<std::uint8_t>>>
   {
     auto [text] = path;
     if (text == "1") {
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
           .set_header(http::field::content_type,
                       http::fields::content_type::plaintext())
           .set_body(self.message());
     }
   };
   
   auto err() -> awaitable<my_error>
   {
     co_return my_error("You will never get anything!");
   }


Built-in Response Converters
================================================================================

+-------------------------+----------------------------------------------------+------------------------------------------------------------------+
|   Response Converter    |                    Description                     |                                                                  |
+=========================+====================================================+==================================================================+
| ``std::string``         | Convert string into ``response`` body.             | ``Content-Type: text/plain`` will be inserted.                   |
+-------------------------+----------------------------------------------------+------------------------------------------------------------------+
| ``std::vector<T>``      | Convert ``std::vector<T>`` into ``response`` body. | ``Content-Type: application/octet-stream`` will be inserted.     |
+-------------------------+----------------------------------------------------+------------------------------------------------------------------+
| ``std::variant<Ts...>`` | Convert any of ``Ts...`` into ``response`` body.   | Note that ``Ts...`` must be convertible to ``response`` as well. |
+-------------------------+----------------------------------------------------+------------------------------------------------------------------+

