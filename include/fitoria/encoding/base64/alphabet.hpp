//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef FITORIA_ENCODING_BASE64_ALPHABET_HPP
#define FITORIA_ENCODING_BASE64_ALPHABET_HPP

#include <fitoria/core/config.hpp>

#include <array>
#include <cstdint>

FITORIA_NAMESPACE_BEGIN

namespace encoding::base64 {

struct standard_alphabet {
  static constexpr char p = '=';
  static constexpr std::array<char, 64> encode_table
      = { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
          'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
          'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
          'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
          '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/' };

  static constexpr std::uint8_t x = 255;
  static constexpr std::array<std::uint8_t, 256> decode_table = {
    x,  x,  x,  x,  x,  x,  x,  x,  x,  x,  x,  x,  x,  x,  x,  x, // 0-15
    x,  x,  x,  x,  x,  x,  x,  x,  x,  x,  x,  x,  x,  x,  x,  x, // 16-31
    x,  x,  x,  x,  x,  x,  x,  x,  x,  x,  x,  62, x,  x,  x,  63, // 32-47
    52, 53, 54, 55, 56, 57, 58, 59, 60, 61, x,  x,  x,  x,  x,  x, // 48-63
    x,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, // 64-79
    15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, x,  x,  x,  x,  x, // 80-95
    x,  26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, // 96-111
    41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, x,  x,  x,  x,  x, // 112-127
    x,  x,  x,  x,  x,  x,  x,  x,  x,  x,  x,  x,  x,  x,  x,  x, // 128-143
    x,  x,  x,  x,  x,  x,  x,  x,  x,  x,  x,  x,  x,  x,  x,  x, // 144-159
    x,  x,  x,  x,  x,  x,  x,  x,  x,  x,  x,  x,  x,  x,  x,  x, // 160-175
    x,  x,  x,  x,  x,  x,  x,  x,  x,  x,  x,  x,  x,  x,  x,  x, // 176-191
    x,  x,  x,  x,  x,  x,  x,  x,  x,  x,  x,  x,  x,  x,  x,  x, // 192-207
    x,  x,  x,  x,  x,  x,  x,  x,  x,  x,  x,  x,  x,  x,  x,  x, // 208-223
    x,  x,  x,  x,  x,  x,  x,  x,  x,  x,  x,  x,  x,  x,  x,  x, // 224-239
    x,  x,  x,  x,  x,  x,  x,  x,  x,  x,  x,  x,  x,  x,  x,  x, // 240-255
  };
};

struct url_alphabet {
  static constexpr char p = '=';
  static constexpr std::array<char, 64> encode_table
      = { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
          'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
          'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
          'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
          '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '-', '_' };

  static constexpr std::uint8_t x = 255;
  static constexpr std::array<std::uint8_t, 256> decode_table = {
    x,  x,  x,  x,  x,  x,  x,  x,  x,  x,  x,  x,  x,  x,  x,  x, // 0-15
    x,  x,  x,  x,  x,  x,  x,  x,  x,  x,  x,  x,  x,  x,  x,  x, // 16-31
    x,  x,  x,  x,  x,  x,  x,  x,  x,  x,  x,  x,  x,  62, x,  x, // 32-47
    52, 53, 54, 55, 56, 57, 58, 59, 60, 61, x,  x,  x,  x,  x,  x, // 48-63
    x,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, // 64-79
    15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, x,  x,  x,  x,  63, // 80-95
    x,  26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, // 96-111
    41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, x,  x,  x,  x,  x, // 112-127
    x,  x,  x,  x,  x,  x,  x,  x,  x,  x,  x,  x,  x,  x,  x,  x, // 128-143
    x,  x,  x,  x,  x,  x,  x,  x,  x,  x,  x,  x,  x,  x,  x,  x, // 144-159
    x,  x,  x,  x,  x,  x,  x,  x,  x,  x,  x,  x,  x,  x,  x,  x, // 160-175
    x,  x,  x,  x,  x,  x,  x,  x,  x,  x,  x,  x,  x,  x,  x,  x, // 176-191
    x,  x,  x,  x,  x,  x,  x,  x,  x,  x,  x,  x,  x,  x,  x,  x, // 192-207
    x,  x,  x,  x,  x,  x,  x,  x,  x,  x,  x,  x,  x,  x,  x,  x, // 208-223
    x,  x,  x,  x,  x,  x,  x,  x,  x,  x,  x,  x,  x,  x,  x,  x, // 224-239
    x,  x,  x,  x,  x,  x,  x,  x,  x,  x,  x,  x,  x,  x,  x,  x, // 240-255
  };
};
}

FITORIA_NAMESPACE_END

#endif
