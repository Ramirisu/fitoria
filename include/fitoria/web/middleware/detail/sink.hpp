//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef FITORIA_WEB_MIDDLEWARE_DETAIL_SINK_HPP
#define FITORIA_WEB_MIDDLEWARE_DETAIL_SINK_HPP

#if defined(FITORIA_HAS_ZLIB)

#include <fitoria/core/config.hpp>

#include <fitoria/web/http_context.hpp>
#include <fitoria/web/http_response.hpp>

#if defined(FITORIA_CXX_COMPILER_GCC)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-copy"
#pragma GCC diagnostic ignored "-Wrestrict"
#endif

#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/iostreams/filtering_stream.hpp>

#if defined(FITORIA_CXX_COMPILER_GCC)
#pragma GCC diagnostic pop
#endif

#include <iterator>

FITORIA_NAMESPACE_BEGIN

namespace middleware::detail {

template <typename Container>
class sink {
public:
  using char_type = char;
  using category = boost::iostreams::sink_tag;

  sink(Container& container)
      : container_(container)
  {
  }

  std::streamsize write(const char* s, std::streamsize n)
  {
    std::copy(s, s + n, std::back_inserter(container_));
    return n;
  }

private:
  Container& container_;
};

}

FITORIA_NAMESPACE_END

#endif

#endif
