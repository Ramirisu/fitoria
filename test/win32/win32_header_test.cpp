//
// Copyright (c) 2022 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#if defined(_WIN32)
#include <Windows.h>
#endif

#include <fitoria/http_server.hpp>

using namespace fitoria;

int main()
{
  auto server = http_server(http_server_config());
  server.run();
}
