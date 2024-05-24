********************************************************************************
State
********************************************************************************

fitoria provides a way to share data between multiple ``route`` s under the same ``scope`` by configuring the data via ``scope::use_state(State&&)``. 

`State Example <https://github.com/Ramirisu/fitoria/blob/main/example/web/state.cpp>`_

.. code-block:: cpp

   namespace cache {
   class simple_cache {
     using map_type = unordered_string_map<std::string>;
   
   public:
     optional<map_type::mapped_type> get(std::string_view key) const
     {
       auto lock = std::shared_lock { mutex_ };
       if (auto it = map_.find(key); it != map_.end()) {
         return it->second;
       }
   
       return nullopt;
     }
   
     bool put(const std::string& key, std::string value)
     {
       auto lock = std::unique_lock { mutex_ };
       return map_.insert_or_assign(key, std::move(value)).second;
     }
   
   private:
     map_type map_;
     mutable std::shared_mutex mutex_;
   };
   
   using simple_cache_ptr = std::shared_ptr<simple_cache>;
   
   auto put(const request& req) -> awaitable<response>
   {
     auto key = req.path().get("key");
     auto value = req.path().get("value");
     if (!key || !value) {
       co_return response::bad_request().build();
     }
   
     auto cache = req.state<simple_cache_ptr>();
     if (!cache) {
       co_return response::internal_server_error().build();
     }
   
     if ((*cache)->put(*key, *value)) {
       co_return response::created().build();
     } else {
       co_return response::accepted().build();
     }
   }
   
   auto get(const request& req) -> awaitable<response>
   {
     auto key = req.path().get("key");
     if (!key) {
       co_return response::bad_request().build();
     }
   
     auto cache = req.state<simple_cache_ptr>();
     if (!cache) {
       co_return response::internal_server_error().build();
     }
   
     if (auto value = (*cache)->get(*key); value) {
       co_return response::ok()
           .set_header(http::field::content_type,
                       http::fields::content_type::plaintext())
           .set_body(*value);
     } else {
       co_return response::not_found().build();
     }
   }
   
   }
   
   int main()
   {
     auto cache = std::make_shared<cache::simple_cache_ptr>();
   
     auto ioc = net::io_context();
     auto server = http_server::builder(ioc)
                       .serve(scope<"/cache">()
                                  .use_state(cache)
                                  .serve(route::put<"/{key}/{value}">(cache::put))
                                  .serve(route::get<"/{key}">(cache::get)))
                       .build();
     server.bind("127.0.0.1", 8080);
   
     ioc.run();
   }
