//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_ENCODING_BASE64_DECODER_HPP
#define FITORIA_ENCODING_BASE64_DECODER_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/encoding/base64/alphabet.hpp>

#include <iterator>
#include <stdexcept>

FITORIA_NAMESPACE_BEGIN

namespace encoding::base64 {

template <typename Alphabet>
class basic_decoder {
  enum class state {
    s0,
    s1,
    s2,
    s3,
    end1,
    end0,
    error,
  };

public:
  bool is_error() const noexcept
  {
    return state_ == state::error;
  }

  bool is_done() const noexcept
  {
    return !is_error() && state_ != state::s1;
  }

  template <typename InputIt, typename OutputIt>
  OutputIt decode_next(InputIt first, InputIt last, OutputIt out)
  {
    static_assert(sizeof(typename std::iterator_traits<InputIt>::value_type)
                  == 1);

    while (first != last) {
      out = decode_next(*first, out);
      ++first;
      if (is_error()) {
        break;
      }
    }

    return out;
  }

  template <typename Byte, typename OutputIt>
  OutputIt decode_next(Byte in, OutputIt out)
  {
    static_assert(sizeof(Byte) == 1);

    if (is_error()) {
      return out;
    }

    auto& table = Alphabet::decode_table;

    switch (state_) {
    case state::s0:
      rest_ = table[s(in)];
      if (rest_ == Alphabet::x) {
        state_ = state::error;
      } else {
        state_ = state::s1;
      }
      break;
    case state::s1:
      if (auto curr = table[s(in)]; curr == Alphabet::x) {
        state_ = state::error;
      } else {
        *out = (rest_ << 2) + (curr >> 4);
        ++out;
        rest_ = curr;
        state_ = state::s2;
      }
      break;
    case state::s2:
      if (s(in) == Alphabet::p) {
        state_ = state::end1;
      } else {
        if (auto curr = table[s(in)]; curr == Alphabet::x) {
          state_ = state::error;
        } else {
          *out = ((rest_ & 0x0f) << 4) + (curr >> 2);
          ++out;
          rest_ = curr;
          state_ = state::s3;
        }
      }
      break;
    case state::s3:
      if (s(in) == Alphabet::p) {
        state_ = state::end0;
      } else {
        if (auto curr = table[s(in)]; curr == Alphabet::x) {
          state_ = state::error;
        } else {
          *out = ((rest_ & 0x03) << 6) + curr;
          ++out;
          state_ = state::s0;
        }
      }
      break;
    case state::end1:
      if (s(in) == Alphabet::p) {
        state_ = state::end0;
      } else {
        state_ = state::error;
      }
      break;
    case state::end0:
      [[fallthrough]];
    default:
      state_ = state::error;
      break;
    }

    return out;
  }

  template <typename InputIt, typename OutputIt>
  OutputIt decode(InputIt first, InputIt last, OutputIt out)
  {
    basic_decoder decoder;
    out = decoder.decode_next(first, last, out);
    if (!decoder.is_done()) {
      throw std::invalid_argument("base64 decode error");
    }

    return out;
  }

private:
  template <typename Byte>
  static std::size_t s(Byte byte) noexcept
  {
    return static_cast<std::size_t>(byte);
  }

  state state_ = state::s0;
  std::uint8_t rest_ = 0;
};

using standard_decoder = basic_decoder<standard_alphabet>;
using url_decoder = basic_decoder<url_alphabet>;

}

FITORIA_NAMESPACE_END

#endif
