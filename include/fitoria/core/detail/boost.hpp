//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_CORE_DETAIL_BOOST_HPP
#define FITORIA_CORE_DETAIL_BOOST_HPP

#include <fitoria/core/config.hpp>

#if defined(FITORIA_HAS_CO_AWAIT)
// workaround: fix boost::asio::awaitable not working for clang-cl and clangd
#define BOOST_ASIO_HAS_CO_AWAIT 1
#endif

#if defined(FITORIA_CXX_COMPILER_MSVC)
// boost/asio/buffer.hpp(247): warning C4702: unreachable code
#pragma warning(push)
#pragma warning(disable : 4702)
#endif

#include <boost/asio.hpp>
#include <boost/beast.hpp>

#if defined(FITORIA_HAS_OPENSSL)
#include <boost/asio/ssl.hpp>
#endif

#include <boost/asio/experimental/awaitable_operators.hpp>
#include <boost/asio/experimental/concurrent_channel.hpp>

#include <boost/beast/_experimental/test/stream.hpp>

#if defined(FITORIA_CXX_COMPILER_MSVC)
// boost/asio/buffer.hpp(247): warning C4702: unreachable code
#pragma warning(pop)
#endif

#endif
