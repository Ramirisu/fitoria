fitoria_add_test(NAME test_web_as_form SRCS test_web_as_form.cpp)
fitoria_add_test(NAME test_web_compile_time_path_checker SRCS
                 test_web_compile_time_path_checker.cpp)
fitoria_add_test(NAME test_web_connection_info SRCS
                 test_web_connection_info.cpp)
fitoria_add_test(NAME test_web_error SRCS test_web_error.cpp)
fitoria_add_test(NAME test_web_extractor SRCS test_web_extractor.cpp)
fitoria_add_test(NAME test_web_http_fields SRCS test_web_http_fields.cpp)
fitoria_add_test(NAME test_web_http_response SRCS test_web_http_response.cpp)
fitoria_add_test(NAME test_web_http_server_tls SRCS
                 test_web_http_server_tls.cpp)
fitoria_add_test(NAME test_web_http_server SRCS test_web_http_server.cpp)
fitoria_add_test(NAME test_web_json SRCS test_web_json.cpp)
fitoria_add_test(NAME test_web_path_matcher SRCS test_web_path_matcher.cpp)
fitoria_add_test(NAME test_web_query_map SRCS test_web_query_map.cpp)
fitoria_add_test(NAME test_web_path_info SRCS test_web_path_info.cpp)
fitoria_add_test(NAME test_web_route SRCS test_web_route.cpp)
fitoria_add_test(NAME test_web_router SRCS test_web_router.cpp)
fitoria_add_test(NAME test_web_scope SRCS test_web_scope.cpp)
fitoria_add_test(NAME test_web_share_state SRCS test_web_share_state.cpp)

add_subdirectory(http)
add_subdirectory(middleware)
