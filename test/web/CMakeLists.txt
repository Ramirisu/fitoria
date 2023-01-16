fitoria_add_test(NAME basic_router_group_test SRCS basic_router_group_test.cpp)
fitoria_add_test(NAME basic_router_tree_test SRCS basic_router_tree_test.cpp)
fitoria_add_test(NAME handlers_invoker_test SRCS handlers_invoker_test.cpp)
fitoria_add_test(NAME http_header_test SRCS http_header_test.cpp)
fitoria_add_test(NAME http_message_test SRCS http_message_test.cpp)
fitoria_add_test(NAME http_response_test SRCS http_response_test.cpp)
fitoria_add_test(NAME http_route_test SRCS http_route_test.cpp)
fitoria_add_test(NAME http_server_test SRCS http_server_test.cpp)
fitoria_add_test(NAME http_server_tls_test SRCS http_server_tls_test.cpp)
fitoria_add_test(NAME http_server_unittest_test SRCS
                 http_server_unittest_test.cpp)
fitoria_add_test(NAME middleware_test SRCS middleware_test.cpp)

add_subdirectory(middleware)
