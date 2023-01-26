fitoria_add_test(NAME as_json_test SRCS as_json_test.cpp)
fitoria_add_test(NAME as_form_test SRCS as_form_test.cpp)
fitoria_add_test(NAME basic_chain_invoker_test SRCS
    basic_chain_invoker_test.cpp)
fitoria_add_test(NAME basic_route_test SRCS basic_route_test.cpp)
fitoria_add_test(NAME basic_router_test SRCS basic_router_test.cpp)
fitoria_add_test(NAME basic_scope_test SRCS basic_scope_test.cpp)
fitoria_add_test(NAME http_client_test SRCS http_client_test.cpp)
fitoria_add_test(NAME http_header_test SRCS http_header_test.cpp)
fitoria_add_test(NAME http_response_test SRCS http_response_test.cpp)
fitoria_add_test(NAME http_server_test SRCS http_server_test.cpp)
fitoria_add_test(NAME http_server_tls_test SRCS http_server_tls_test.cpp)
fitoria_add_test(NAME http_server_unittest_test SRCS
    http_server_unittest_test.cpp)
fitoria_add_test(NAME route_params_test SRCS route_params_test.cpp)
fitoria_add_test(NAME segments_view_test SRCS
    segments_view_test.cpp)

add_subdirectory(http)
add_subdirectory(middleware)
