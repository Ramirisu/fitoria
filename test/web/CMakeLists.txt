fitoria_add_test(NAME as_form_test SRCS as_form_test.cpp)
fitoria_add_test(NAME compile_time_path_checker_test SRCS compile_time_path_checker_test.cpp)
fitoria_add_test(NAME http_fields_test SRCS http_fields_test.cpp)
fitoria_add_test(NAME http_response_test SRCS http_response_test.cpp)
fitoria_add_test(NAME http_server_test SRCS http_server_test.cpp)
fitoria_add_test(NAME http_server_tls_test SRCS http_server_tls_test.cpp)
fitoria_add_test(NAME http_server_unittest_test SRCS http_server_unittest_test.cpp)
fitoria_add_test(NAME json_test SRCS json_test.cpp)
fitoria_add_test(NAME pattern_matcher_test SRCS pattern_matcher_test.cpp)
fitoria_add_test(NAME query_map_test SRCS query_map_test.cpp)
fitoria_add_test(NAME route_params_test SRCS route_params_test.cpp)
fitoria_add_test(NAME route_test SRCS route_test.cpp)
fitoria_add_test(NAME router_test SRCS router_test.cpp)
fitoria_add_test(NAME scope_test SRCS scope_test.cpp)

add_subdirectory(http)
add_subdirectory(middleware)
