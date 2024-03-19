//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_LOG_ASYNC_STREAM_FILE_WRITER_HPP
#define FITORIA_LOG_ASYNC_STREAM_FILE_WRITER_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/log/basic_async_stream_file_writer.hpp>

FITORIA_NAMESPACE_BEGIN

namespace log {

#if defined(BOOST_ASIO_HAS_FILE)

using async_stream_file_writer = basic_async_stream_file_writer<false>;

inline std::shared_ptr<async_writer>
make_async_stream_file_writer(const std::string& path)
{
  return std::make_shared<async_stream_file_writer>(
      net::stream_file(net::system_executor(),
                       path,
                       net::file_base::create | net::file_base::write_only
                           | net::file_base::append));
  // TODO: boost::asio::stream_file bug, `append` flag not working
}

#endif

}

FITORIA_NAMESPACE_END

#endif
