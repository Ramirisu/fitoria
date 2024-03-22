fitoria_add_test(NAME test_log_async_stdout_writer SRCS
                 test_log_async_stdout_writer.cpp)
fitoria_try_target_link_liburing(test_log_async_stdout_writer)
fitoria_add_test(NAME test_log_async_stream_file_writer SRCS
                 test_log_async_stream_file_writer.cpp)
fitoria_try_target_link_liburing(test_log_async_stream_file_writer)
fitoria_add_test(NAME test_log_filter SRCS test_log_filter.cpp)
fitoria_add_test(NAME test_log_formatter SRCS test_log_formatter.cpp)
fitoria_add_test(NAME test_log_level SRCS test_log_level.cpp)
fitoria_add_test(NAME test_log_log SRCS test_log_log.cpp)
fitoria_add_test(NAME test_log_sync_stdout_writer SRCS
                 test_log_sync_stdout_writer.cpp)
fitoria_add_test(NAME test_log_sync_stream_file_writer SRCS
                 test_log_sync_stream_file_writer.cpp)
