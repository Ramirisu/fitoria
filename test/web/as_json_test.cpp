//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria_test.h>

#include <fitoria/web/as_json.hpp>

using namespace fitoria;

TEST_SUITE_BEGIN("web.as_json");

namespace {

struct user_t {
  std::string name;
  std::string birth;

  friend bool operator==(const user_t&, const user_t&) = default;
};

user_t tag_invoke(const json::value_to_tag<user_t>&, const json::value& jv)
{
  return user_t {
    .name = std::string(jv.at("name").as_string()),
    .birth = std::string(jv.at("birth").as_string()),
  };
}

void tag_invoke(const json::value_from_tag&,
                json::value& jv,
                const user_t& user)
{
  jv = { { "name", user.name }, { "birth", user.birth } };
}

}

TEST_CASE("body_as_json")
{
  const json::value jv = {
    { "name", "Rina Hidaka" },
    { "birth", "1994/06/15" },
  };
  {
    CHECK_EQ(as_json<user_t>("").error(),
             make_error_code(error::invalid_json_format));
  }
  {
    CHECK_EQ(as_json<user_t>("{").error(),
             make_error_code(error::invalid_json_format));
  }
  {
    auto text = json::serialize(jv);
    CHECK_EQ(as_json<user_t>(text),
             user_t {
                 .name = "Rina Hidaka",
                 .birth = "1994/06/15",
             });
  }
}

TEST_SUITE_END();
