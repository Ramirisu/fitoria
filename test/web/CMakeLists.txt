fitoria_add_test(NAME test_web_as_form SRCS test_web_as_form.cpp)
fitoria_add_test(NAME test_web_async_readable_stream SRCS
                 test_web_async_readable_stream.cpp)
fitoria_try_target_link_liburing(test_web_async_readable_stream)
fitoria_add_test(NAME test_web_connection_info SRCS
                 test_web_connection_info.cpp)
fitoria_add_test(NAME test_web_error SRCS test_web_error.cpp)
fitoria_add_test(NAME test_web_from_http_request SRCS
                 test_web_from_http_request.cpp)
fitoria_add_test(NAME test_web_http_fields SRCS test_web_http_fields.cpp)
fitoria_add_test(NAME test_web_http_response SRCS test_web_http_response.cpp)
fitoria_try_target_link_liburing(test_web_http_response)
fitoria_add_test(NAME test_web_http_server_tls SRCS
                 test_web_http_server_tls.cpp)
fitoria_add_test(NAME test_web_http_server SRCS test_web_http_server.cpp)
fitoria_add_test(NAME test_web_json SRCS test_web_json.cpp)
fitoria_add_test(NAME test_web_path_matcher SRCS test_web_path_matcher.cpp)
fitoria_add_test(NAME test_web_path_parser SRCS test_web_path_parser.cpp)
fitoria_add_test(NAME test_web_query_map SRCS test_web_query_map.cpp)
fitoria_add_test(NAME test_web_path_info SRCS test_web_path_info.cpp)
fitoria_add_test(NAME test_web_route SRCS test_web_route.cpp)
fitoria_add_test(NAME test_web_router SRCS test_web_router.cpp)
fitoria_add_test(NAME test_web_scope SRCS test_web_scope.cpp)
fitoria_add_test(NAME test_web_state SRCS test_web_state.cpp)
fitoria_add_test(NAME test_web_stream_file SRCS test_web_stream_file.cpp)
fitoria_try_target_link_liburing(test_web_stream_file)
fitoria_add_test(NAME test_web_to_http_response SRCS
                 test_web_to_http_response.cpp)
fitoria_try_target_link_liburing(test_web_to_http_response)

add_subdirectory(http)
add_subdirectory(middleware)
