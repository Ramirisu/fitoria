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

json::result_for<user_t, json::value>::type
tag_invoke(const json::try_value_to_tag<user_t>&, const json::value& jv)
{
  user_t user;

  if (!jv.is_object()) {
    return make_error_code(json::error::incomplete);
  }

  const auto& obj = jv.get_object();

  auto* name = obj.if_contains("name");
  auto* birth = obj.if_contains("birth");
  if (name && birth && name->is_string() && birth->is_string()) {
    return user_t { .name = std::string(name->get_string()),
                    .birth = std::string(birth->get_string()) };
  }

  return make_error_code(json::error::incomplete);
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
             make_error_code(json::error::incomplete));
  }
  {
    CHECK_EQ(as_json<user_t>("{").error(),
             make_error_code(json::error::incomplete));
  }
  {
    CHECK_EQ(as_json<user_t>(R"del({ "name": "Rina Hidaka" })del").error(),
             make_error_code(json::error::incomplete));
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
