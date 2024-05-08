fitoria_add_test(NAME test_dynamic_buffer SRCS test_dynamic_buffer.cpp)
fitoria_add_test(NAME test_fixed_string SRCS test_fixed_string.cpp)
fitoria_add_test(NAME test_from_string SRCS test_from_string.cpp)
fitoria_add_test(NAME test_net_io_context SRCS test_net_io_context.cpp)
fitoria_add_test(NAME test_net SRCS test_net.cpp)
fitoria_add_test(NAME test_type_traits SRCS test_type_traits.cpp)

add_subdirectory(expected)
add_subdirectory(optional)
