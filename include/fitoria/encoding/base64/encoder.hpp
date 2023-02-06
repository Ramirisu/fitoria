//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef FITORIA_ENCODING_BASE64_ENCODER_HPP
#define FITORIA_ENCODING_BASE64_ENCODER_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/encoding/base64/alphabet.hpp>

#include <iterator>
#include <stdexcept>

FITORIA_NAMESPACE_BEGIN

namespace encoding::base64 {

enum class padding {
  no = 0,
  yes,
};

template <typename Alphabet>
class basic_encoder {
  enum class state {
    s0,
    s1,
    s2,
  };

public:
  basic_encoder() = default;

  basic_encoder(padding padding)
      : padding_(padding)
  {
  }

  template <typename InputIt, typename OutputIt>
  OutputIt encode_next(InputIt first, InputIt last, OutputIt out)
  {
    static_assert(sizeof(typename std::iterator_traits<InputIt>::value_type)
                  == 1);

    while (first != last) {
      out = encode_next(*first, out);
      ++first;
    }

    return out;
  }

  template <typename Byte, typename OutputIt>
  OutputIt encode_next(Byte in, OutputIt out)
  {
    static_assert(sizeof(Byte) == 1);

    auto& table = Alphabet::encode_table;

    switch (state_) {
    case state::s2:
      *out = table[((rest_ & 0x0f) << 2) + (u8(in) >> 6)];
      ++out;
      *out = table[(u8(in) & 0x3f)];
      ++out;
      state_ = state::s0;
      break;
    case state::s1:
      *out = table[((rest_ & 0x03) << 4) + (u8(in) >> 4)];
      ++out;
      rest_ = u8(in);
      state_ = state::s2;
      break;
    case state::s0:
    default:
      *out = table[(u8(in) >> 2)];
      ++out;
      rest_ = u8(in);
      state_ = state::s1;
      break;
    }

    return out;
  }

  template <typename OutputIt>
  OutputIt encode_rest(OutputIt out)
  {
    auto& table = Alphabet::encode_table;

    switch (state_) {
    case state::s2:
      *out = table[((rest_ & 0x0f) << 2)];
      ++out;
      if (padding_ == padding::yes) {
        *out = Alphabet::p;
        ++out;
      }
      state_ = state::s0;
      break;
    case state::s1:
      *out = table[((rest_ & 0x03) << 4)];
      ++out;
      if (padding_ == padding::yes) {
        *out = Alphabet::p;
        ++out;
        *out = Alphabet::p;
        ++out;
      }
      state_ = state::s0;
      break;
    case state::s0:
      [[fallthrough]];
    default:
      break;
    }

    return out;
  }

  template <typename InputIt, typename OutputIt>
  OutputIt encode(InputIt first, InputIt last, OutputIt out)
  {
    out = encode_next(first, last, out);
    out = encode_rest(out);
    return out;
  }

private:
  template <typename Byte>
  std::uint8_t u8(Byte byte)
  {
    return static_cast<std::uint8_t>(byte);
  }

  padding padding_ = padding::yes;
  state state_ = state::s0;
  std::uint8_t rest_ = 0;
};

using standard_encoder = basic_encoder<standard_alphabet>;
using url_encoder = basic_encoder<url_alphabet>;

}

FITORIA_NAMESPACE_END

#endif
