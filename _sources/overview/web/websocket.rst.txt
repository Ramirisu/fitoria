********************************************************************************
WebSocket
********************************************************************************

fitoria supports websocket by using the ``websocket`` extractor. 

`WebSocket Example <https://github.com/Ramirisu/fitoria/blob/main/example/web/websocket.cpp>`_

.. code-block:: cpp

   auto ws_handler(websocket::context& ctx) -> awaitable<void>
   {
     for (auto msg = co_await ctx.async_read(); msg;
          msg = co_await ctx.async_read()) {
       if (auto binary = std::get_if<websocket::binary_t>(&*msg); binary) {
         co_await ctx.async_write_binary(
             std::span { binary->value.data(), binary->value.size() });
       } else if (auto text = std::get_if<websocket::text_t>(&*msg); text) {
         co_await ctx.async_write_text(text->value);
       } else if (auto c = std::get_if<websocket::close_t>(&*msg); c) {
         break;
       }
     }
   }
   
   auto http_handler(websocket ws) -> awaitable<response>
   {
     ws.set_handshake_timeout(std::chrono::seconds(5));
     ws.set_idle_timeout(std::chrono::seconds(30));
     ws.set_keep_alive_pings(true);
     co_return ws.set_handler(ws_handler);
   }
   
   int main()
   {
     auto ioc = net::io_context();
     auto server = http_server::builder(ioc)
                       .serve(route::get<"/ws">(http_handler))
                       .build();
     server.bind("127.0.0.1", 8080);
   
     ioc.run();
   }
