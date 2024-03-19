//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_LOG_ASYNC_STDOUT_WRITER_HPP
#define FITORIA_LOG_ASYNC_STDOUT_WRITER_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/log/basic_async_stream_file_writer.hpp>

FITORIA_NAMESPACE_BEGIN

namespace log {

#if defined(BOOST_ASIO_HAS_FILE)

using async_stdout_writer = basic_async_stream_file_writer<true>;

inline std::shared_ptr<async_writer> make_async_stdout_writer()
{
#if defined(FITORIA_TARGET_WINDOWS)
  return std::make_shared<async_stdout_writer>(net::stream_file(
      net::system_executor(), "CONOUT$", net::file_base::write_only));
#else
  return std::make_shared<async_stdout_writer>(
      net::stream_file(net::system_executor(), STDOUT_FILENO));
#endif
}

#endif

}

FITORIA_NAMESPACE_END

#endif
