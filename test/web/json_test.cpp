//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria_test.h>

#include <fitoria/web/json.hpp>

using namespace fitoria;
using namespace fitoria::web;
using namespace fitoria::web::detail;

TEST_SUITE_BEGIN("web.json");

namespace {

struct user_t {
  std::string name;
  std::string birth;

  friend bool operator==(const user_t&, const user_t&) = default;
};

boost::json::result_for<user_t, boost::json::value>::type
tag_invoke(const boost::json::try_value_to_tag<user_t>&,
           const boost::json::value& jv)
{
  user_t user;

  if (!jv.is_object()) {
    return make_error_code(boost::json::error::incomplete);
  }

  const auto& obj = jv.get_object();

  auto* name = obj.if_contains("name");
  auto* birth = obj.if_contains("birth");
  if (name && birth && name->is_string() && birth->is_string()) {
    return user_t { .name = std::string(name->get_string()),
                    .birth = std::string(birth->get_string()) };
  }

  return make_error_code(boost::json::error::incomplete);
}

}

TEST_CASE("as_json")
{
  const boost::json::value jv = {
    { "name", "Rina Hidaka" },
    { "birth", "1994/06/15" },
  };
  {
    CHECK_EQ(as_json<user_t>("").error(),
             make_error_code(boost::json::error::incomplete));
  }
  {
    CHECK_EQ(as_json<user_t>("{").error(),
             make_error_code(boost::json::error::incomplete));
  }
  {
    CHECK_EQ(as_json<user_t>(R"del({ "name": "Rina Hidaka" })del").error(),
             make_error_code(boost::json::error::incomplete));
  }
  {
    auto text = boost::json::serialize(jv);
    CHECK_EQ(as_json<user_t>(text),
             user_t {
                 .name = "Rina Hidaka",
                 .birth = "1994/06/15",
             });
  }
}

TEST_SUITE_END();
