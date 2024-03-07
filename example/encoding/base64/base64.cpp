//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria/encoding/base64.hpp>

#include <iostream>
#include <stdexcept>
#include <string>

using namespace fitoria::encoding::base64;

void encode_with_padding()
{
  const auto input = std::string_view("Hello World");
  std::string out;
  standard_encoder encoder;
  encoder.encode(input.begin(), input.end(), std::back_inserter(out));
  std::cout << out << "\n"; // "SGVsbG8gV29ybGQ="
}

void encode_with_no_padding()
{
  const auto input = std::string_view("Hello World");
  std::string out;
  standard_encoder encoder(padding::no);
  encoder.encode(input.begin(), input.end(), std::back_inserter(out));
  std::cout << out << "\n"; // "SGVsbG8gV29ybGQ"
}

void encode_buffer_oriented()
{
  std::string line;
  std::string out;
  standard_encoder encoder;
  while (std::getline(std::cin, line) && !line.empty()) {
    encoder.encode_next(line.begin(), line.end(), std::back_inserter(out));
  }
  encoder.encode_rest(std::back_inserter(out));
  std::cout << out << "\n";
}

void decode()
{
  const auto input = std::string_view("SGVsbG8gV29ybGQ=");
  std::string out;
  standard_decoder decoder;
  decoder.decode(input.begin(), input.end(), std::back_inserter(out));
  std::cout << out << "\n"; // "Hello World"
}

void decode_buffer_oriented()
{
  std::string line;
  std::string out;
  standard_decoder decoder;
  while (!decoder.is_error() && std::getline(std::cin, line) && !line.empty()) {
    decoder.decode_next(line.begin(), line.end(), std::back_inserter(out));
  }
  if (!decoder.is_done()) {
    throw std::invalid_argument("base64 decode error");
  }
  std::cout << out << "\n";
}

int main()
{
  encode_with_padding();
  encode_with_no_padding();
  encode_buffer_oriented();
  decode();
  decode_buffer_oriented();
}
