//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#if defined(_WIN32)
#include <Windows.h>
#endif

#include <fitoria/fitoria.hpp>

using namespace fitoria;

int main()
{
  auto server = http_server::builder().build();
  server.run();
}
