//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria/test/test.hpp>

#include <fitoria/test/http_server_utils.hpp>

#include <fitoria/web/async_read_until_eof.hpp>
#include <fitoria/web/response.hpp>

using namespace fitoria;
using namespace fitoria::web;
using namespace fitoria::test;

TEST_SUITE_BEGIN("[fitoria.web.response]");

template <typename Builder>
void test_builder(Builder&& builder)
{
  auto res = std::forward<Builder>(builder)
                 .set_status(http::status::internal_server_error)
                 .set_version(http::version::v0_9)
                 .set_header(http::field::content_type, "text/plain")
                 .set_header("Content-Encoding", "deflate")
                 .insert_header(http::field::accept_encoding, "gzip")
                 .insert_header("Range", "bytes=0-99")
                 .build();
  CHECK_EQ(res.status(), http::status::internal_server_error);
  CHECK_EQ(res.version(), http::version::v0_9);
  CHECK_EQ(res.headers().get("Content-Type"), "text/plain");
  CHECK_EQ(res.headers().get(http::field::content_encoding), "deflate");
  CHECK_EQ(res.headers().get("Accept-Encoding"), "gzip");
  CHECK_EQ(res.headers().get(http::field::range), "bytes=0-99");
}

TEST_CASE("builder with lvalue")
{
  auto builder = response_builder(http::status::ok);
  test_builder(builder);
}

TEST_CASE("builder with rvalue")
{
  test_builder(response_builder(http::status::ok));
}

TEST_CASE("copy/move")
{
  {
    auto s = response::internal_server_error().build();
    response d(s);
    CHECK_EQ(d.status(), http::status::internal_server_error);
  }
  {
    auto s = response::internal_server_error().build();
    auto d = s;
    CHECK_EQ(d.status(), http::status::internal_server_error);
  }
  {
    auto s = response::internal_server_error().build();
    response d(std::move(s));
    CHECK_EQ(d.status(), http::status::internal_server_error);
  }
  {
    auto s = response::internal_server_error().build();
    auto d = std::move(s);
    CHECK_EQ(d.status(), http::status::internal_server_error);
  }
}

TEST_CASE("status")
{
  CHECK_EQ(response::ok().build().status(), //
           http::status::ok);
  CHECK_EQ(response::continue_().build().status(), //
           http::status::continue_);
  CHECK_EQ(response::switching_protocols().build().status(), //
           http::status::switching_protocols);
  CHECK_EQ(response::processing().build().status(), //
           http::status::processing);
  CHECK_EQ(response::early_hints().build().status(), //
           http::status::early_hints);
  CHECK_EQ(response::ok().build().status(), //
           http::status::ok);
  CHECK_EQ(response::created().build().status(), //
           http::status::created);
  CHECK_EQ(response::accepted().build().status(), //
           http::status::accepted);
  CHECK_EQ(response::non_authoritative_information().build().status(), //
           http::status::non_authoritative_information);
  CHECK_EQ(response::no_content().build().status(), //
           http::status::no_content);
  CHECK_EQ(response::reset_content().build().status(), //
           http::status::reset_content);
  CHECK_EQ(response::partial_content().build().status(), //
           http::status::partial_content);
  CHECK_EQ(response::multi_status().build().status(), //
           http::status::multi_status);
  CHECK_EQ(response::already_reported().build().status(), //
           http::status::already_reported);
  CHECK_EQ(response::im_used().build().status(), //
           http::status::im_used);
  CHECK_EQ(response::multiple_choices().build().status(), //
           http::status::multiple_choices);
  CHECK_EQ(response::moved_permanently().build().status(), //
           http::status::moved_permanently);
  CHECK_EQ(response::found().build().status(), //
           http::status::found);
  CHECK_EQ(response::see_other().build().status(), //
           http::status::see_other);
  CHECK_EQ(response::not_modified().build().status(), //
           http::status::not_modified);
  CHECK_EQ(response::use_proxy().build().status(), //
           http::status::use_proxy);
  CHECK_EQ(response::temporary_redirect().build().status(), //
           http::status::temporary_redirect);
  CHECK_EQ(response::permanent_redirect().build().status(), //
           http::status::permanent_redirect);
  CHECK_EQ(response::bad_request().build().status(), //
           http::status::bad_request);
  CHECK_EQ(response::unauthorized().build().status(), //
           http::status::unauthorized);
  CHECK_EQ(response::payment_required().build().status(), //
           http::status::payment_required);
  CHECK_EQ(response::forbidden().build().status(), //
           http::status::forbidden);
  CHECK_EQ(response::not_found().build().status(), //
           http::status::not_found);
  CHECK_EQ(response::method_not_allowed().build().status(), //
           http::status::method_not_allowed);
  CHECK_EQ(response::not_acceptable().build().status(), //
           http::status::not_acceptable);
  CHECK_EQ(response::proxy_authentication_required().build().status(), //
           http::status::proxy_authentication_required);
  CHECK_EQ(response::request_timeout().build().status(), //
           http::status::request_timeout);
  CHECK_EQ(response::conflict().build().status(), //
           http::status::conflict);
  CHECK_EQ(response::gone().build().status(), //
           http::status::gone);
  CHECK_EQ(response::length_required().build().status(), //
           http::status::length_required);
  CHECK_EQ(response::precondition_failed().build().status(), //
           http::status::precondition_failed);
  CHECK_EQ(response::payload_too_large().build().status(), //
           http::status::payload_too_large);
  CHECK_EQ(response::uri_too_long().build().status(), //
           http::status::uri_too_long);
  CHECK_EQ(response::unsupported_media_type().build().status(), //
           http::status::unsupported_media_type);
  CHECK_EQ(response::range_not_satisfiable().build().status(), //
           http::status::range_not_satisfiable);
  CHECK_EQ(response::expectation_failed().build().status(), //
           http::status::expectation_failed);
  CHECK_EQ(response::misdirected_request().build().status(), //
           http::status::misdirected_request);
  CHECK_EQ(response::unprocessable_entity().build().status(), //
           http::status::unprocessable_entity);
  CHECK_EQ(response::locked().build().status(), //
           http::status::locked);
  CHECK_EQ(response::failed_dependency().build().status(), //
           http::status::failed_dependency);
  CHECK_EQ(response::too_early().build().status(), //
           http::status::too_early);
  CHECK_EQ(response::upgrade_required().build().status(), //
           http::status::upgrade_required);
  CHECK_EQ(response::precondition_required().build().status(), //
           http::status::precondition_required);
  CHECK_EQ(response::too_many_requests().build().status(), //
           http::status::too_many_requests);
  CHECK_EQ(response::request_header_fields_too_large().build().status(), //
           http::status::request_header_fields_too_large);
  CHECK_EQ(response::unavailable_for_legal_reasons().build().status(), //
           http::status::unavailable_for_legal_reasons);
  CHECK_EQ(response::internal_server_error().build().status(), //
           http::status::internal_server_error);
  CHECK_EQ(response::not_implemented().build().status(), //
           http::status::not_implemented);
  CHECK_EQ(response::bad_gateway().build().status(), //
           http::status::bad_gateway);
  CHECK_EQ(response::service_unavailable().build().status(), //
           http::status::service_unavailable);
  CHECK_EQ(response::gateway_timeout().build().status(), //
           http::status::gateway_timeout);
  CHECK_EQ(response::http_version_not_supported().build().status(), //
           http::status::http_version_not_supported);
  CHECK_EQ(response::variant_also_negotiates().build().status(), //
           http::status::variant_also_negotiates);
  CHECK_EQ(response::insufficient_storage().build().status(), //
           http::status::insufficient_storage);
  CHECK_EQ(response::loop_detected().build().status(), //
           http::status::loop_detected);
  CHECK_EQ(response::not_extended().build().status(), //
           http::status::not_extended);
  CHECK_EQ(response::network_authentication_required().build().status(), //
           http::status::network_authentication_required);
}

TEST_CASE("set_body")
{
  sync_wait([]() -> awaitable<void> {
    const auto body = std::string_view("Hello World!");
    auto res = response::ok().set_body(body);
    CHECK_EQ(std::get<any_body::sized>(res.body().size()),
             any_body::sized { body.size() });
    CHECK_EQ(co_await async_read_until_eof<std::string>(res.body().stream()),
             body);
  });
}

struct user_t {
  std::string name;

  friend bool operator==(const user_t&, const user_t&) = default;
};

void tag_invoke(const boost::json::value_from_tag&,
                boost::json::value& jv,
                const user_t& user)
{
  jv = {
    { "name", user.name },
  };
}

TEST_CASE("set_json")
{
  sync_wait([]() -> awaitable<void> {
    {
      auto res = response::ok().set_json({ { "name", "Rina Hidaka" } });
      CHECK_EQ(co_await async_read_until_eof<std::string>(res.body().stream()),
               R"({"name":"Rina Hidaka"})");
    }
    {
      auto res = response::ok().set_json(user_t { .name = "Rina Hidaka" });
      CHECK_EQ(co_await async_read_until_eof<std::string>(res.body().stream()),
               R"({"name":"Rina Hidaka"})");
    }
  });
}

TEST_SUITE_END();
