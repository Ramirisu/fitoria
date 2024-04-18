//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_WEB_RESPONSE_HPP
#define FITORIA_WEB_RESPONSE_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/core/json.hpp>

#include <fitoria/web/any_async_readable_stream.hpp>
#include <fitoria/web/async_readable_vector_stream.hpp>
#include <fitoria/web/http.hpp>
#include <fitoria/web/http_body.hpp>
#include <fitoria/web/http_fields.hpp>

#include <span>

FITORIA_NAMESPACE_BEGIN

namespace web {

class response_builder;

class response {
  friend class response_builder;

  http::status_code status_code_ = http::status::ok;
  http_fields fields_;
  http_body body_;

  response(http::status_code status_code, http_fields fields, http_body body)
      : status_code_(status_code)
      , fields_(std::move(fields))
      , body_(std::move(body))
  {
  }

public:
  response() = default;

  response(const response&) = delete;

  response& operator=(const response&) = delete;

  response(response&&) = default;

  response& operator=(response&&) = default;

  auto status_code() const noexcept -> http::status_code
  {
    return status_code_;
  }

  auto fields() noexcept -> http_fields&
  {
    return fields_;
  }

  auto fields() const noexcept -> const http_fields&
  {
    return fields_;
  }

  auto body() noexcept -> http_body&
  {
    return body_;
  }

  auto body() const noexcept -> const http_body&
  {
    return body_;
  }

  auto builder() -> response_builder;

  static auto continue_() -> response_builder;
  static auto switching_protocols() -> response_builder;
  static auto processing() -> response_builder;
  static auto ok() -> response_builder;
  static auto created() -> response_builder;
  static auto accepted() -> response_builder;
  static auto non_authoritative_information() -> response_builder;
  static auto no_content() -> response_builder;
  static auto reset_content() -> response_builder;
  static auto partial_content() -> response_builder;
  static auto multi_status() -> response_builder;
  static auto already_reported() -> response_builder;
  static auto im_used() -> response_builder;
  static auto multiple_choices() -> response_builder;
  static auto moved_permanently() -> response_builder;
  static auto found() -> response_builder;
  static auto see_other() -> response_builder;
  static auto not_modified() -> response_builder;
  static auto use_proxy() -> response_builder;
  static auto temporary_redirect() -> response_builder;
  static auto permanent_redirect() -> response_builder;
  static auto bad_request() -> response_builder;
  static auto unauthorized() -> response_builder;
  static auto payment_required() -> response_builder;
  static auto forbidden() -> response_builder;
  static auto not_found() -> response_builder;
  static auto method_not_allowed() -> response_builder;
  static auto not_acceptable() -> response_builder;
  static auto proxy_authentication_required() -> response_builder;
  static auto request_timeout() -> response_builder;
  static auto conflict() -> response_builder;
  static auto gone() -> response_builder;
  static auto length_required() -> response_builder;
  static auto precondition_failed() -> response_builder;
  static auto payload_too_large() -> response_builder;
  static auto uri_too_long() -> response_builder;
  static auto unsupported_media_type() -> response_builder;
  static auto range_not_satisfiable() -> response_builder;
  static auto expectation_failed() -> response_builder;
  static auto misdirected_request() -> response_builder;
  static auto unprocessable_entity() -> response_builder;
  static auto locked() -> response_builder;
  static auto failed_dependency() -> response_builder;
  static auto upgrade_required() -> response_builder;
  static auto precondition_required() -> response_builder;
  static auto too_many_requests() -> response_builder;
  static auto request_header_fields_too_large() -> response_builder;
  static auto connection_closed_without_response() -> response_builder;
  static auto unavailable_for_legal_reasons() -> response_builder;
  static auto client_closed_request() -> response_builder;
  static auto internal_server_error() -> response_builder;
  static auto not_implemented() -> response_builder;
  static auto bad_gateway() -> response_builder;
  static auto service_unavailable() -> response_builder;
  static auto gateway_timeout() -> response_builder;
  static auto http_version_not_supported() -> response_builder;
  static auto variant_also_negotiates() -> response_builder;
  static auto insufficient_storage() -> response_builder;
  static auto loop_detected() -> response_builder;
  static auto not_extended() -> response_builder;
  static auto network_authentication_required() -> response_builder;
  static auto network_connect_timeout_error() -> response_builder;
};

class response_builder {
  friend class response;

  http::status_code status_code_ = http::status::ok;
  http_fields fields_;
  http_body body_;

  response_builder(http::status_code status_code,
                   http_fields fields,
                   http_body body)
      : status_code_(status_code)
      , fields_(std::move(fields))
      , body_(std::move(body))
  {
  }

public:
  explicit response_builder(http::status status)
      : status_code_(status)
  {
  }

  response_builder(const response_builder&) = delete;

  response_builder& operator=(const response_builder&) = delete;

  response_builder(response_builder&&) = default;

  response_builder& operator=(response_builder&&) = default;

  auto set_status_code(http::status_code status_code) & noexcept
      -> response_builder&
  {
    status_code_ = status_code;
    return *this;
  }

  auto set_status_code(http::status_code status_code) && noexcept
      -> response_builder&&
  {
    status_code_ = status_code;
    return std::move(*this);
  }

  auto set_field(std::string_view name,
                 std::string_view value) & -> response_builder&
  {
    fields_.set(name, value);
    return *this;
  }

  auto set_field(std::string_view name,
                 std::string_view value) && -> response_builder&&
  {
    fields_.set(name, value);
    return std::move(*this);
  }

  auto set_field(http::field name,
                 std::string_view value) & -> response_builder&
  {
    fields_.set(name, value);
    return *this;
  }

  auto set_field(http::field name,
                 std::string_view value) && -> response_builder&&
  {
    fields_.set(name, value);
    return std::move(*this);
  }

  auto insert_field(std::string_view name,
                    std::string_view value) & -> response_builder&
  {
    fields_.insert(name, value);
    return *this;
  }

  auto insert_field(std::string_view name,
                    std::string_view value) && -> response_builder&&
  {
    fields_.insert(name, value);
    return std::move(*this);
  }

  auto insert_field(http::field name,
                    std::string_view value) & -> response_builder&
  {
    fields_.insert(name, value);
    return *this;
  }

  auto insert_field(http::field name,
                    std::string_view value) && -> response_builder&&
  {
    fields_.insert(name, value);
    return std::move(*this);
  }

  auto build() -> response
  {
    return response(status_code_, std::move(fields_), std::move(body_));
  }

  template <std::size_t N>
  auto set_body(std::span<const std::byte, N> bytes) -> response
  {
    body_ = http_body(http_body::sized { bytes.size() },
                      async_readable_vector_stream(bytes));
    return build();
  }

  auto set_body(std::string_view sv) -> response
  {
    return set_body(std::as_bytes(std::span(sv.begin(), sv.end())));
  }

  auto set_json(const boost::json::value& jv) -> response
  {
    set_field(http::field::content_type, http::fields::content_type::json());
    auto str = boost::json::serialize(jv);
    return set_body(std::as_bytes(std::span(str.begin(), str.end())));
  }

  template <typename T>
    requires(boost::json::has_value_from<T>::value)
  auto set_json(const T& obj) -> response
  {
    return set_json(boost::json::value_from(obj));
  }

  template <async_readable_stream AsyncReadableStream>
  auto set_stream(AsyncReadableStream&& stream) -> response
  {
    body_ = http_body(http_body::chunked(),
                      std::forward<AsyncReadableStream>(stream));
    return build();
  }
};

inline auto response::builder() -> response_builder
{
  return response_builder(status_code_, std::move(fields_), std::move(body_));
}

inline auto response::continue_() -> response_builder
{
  return response_builder(http::status::continue_);
}

inline auto response::switching_protocols() -> response_builder
{
  return response_builder(http::status::switching_protocols);
}

inline auto response::processing() -> response_builder
{
  return response_builder(http::status::processing);
}

inline auto response::ok() -> response_builder
{
  return response_builder(http::status::ok);
}

inline auto response::created() -> response_builder
{
  return response_builder(http::status::created);
}

inline auto response::accepted() -> response_builder
{
  return response_builder(http::status::accepted);
}

inline auto response::non_authoritative_information() -> response_builder
{
  return response_builder(http::status::non_authoritative_information);
}

inline auto response::no_content() -> response_builder
{
  return response_builder(http::status::no_content);
}

inline auto response::reset_content() -> response_builder
{
  return response_builder(http::status::reset_content);
}

inline auto response::partial_content() -> response_builder
{
  return response_builder(http::status::partial_content);
}

inline auto response::multi_status() -> response_builder
{
  return response_builder(http::status::multi_status);
}

inline auto response::already_reported() -> response_builder
{
  return response_builder(http::status::already_reported);
}

inline auto response::im_used() -> response_builder
{
  return response_builder(http::status::im_used);
}

inline auto response::multiple_choices() -> response_builder
{
  return response_builder(http::status::multiple_choices);
}

inline auto response::moved_permanently() -> response_builder
{
  return response_builder(http::status::moved_permanently);
}

inline auto response::found() -> response_builder
{
  return response_builder(http::status::found);
}

inline auto response::see_other() -> response_builder
{
  return response_builder(http::status::see_other);
}

inline auto response::not_modified() -> response_builder
{
  return response_builder(http::status::not_modified);
}

inline auto response::use_proxy() -> response_builder
{
  return response_builder(http::status::use_proxy);
}

inline auto response::temporary_redirect() -> response_builder
{
  return response_builder(http::status::temporary_redirect);
}

inline auto response::permanent_redirect() -> response_builder
{
  return response_builder(http::status::permanent_redirect);
}

inline auto response::bad_request() -> response_builder
{
  return response_builder(http::status::bad_request);
}

inline auto response::unauthorized() -> response_builder
{
  return response_builder(http::status::unauthorized);
}

inline auto response::payment_required() -> response_builder
{
  return response_builder(http::status::payment_required);
}

inline auto response::forbidden() -> response_builder
{
  return response_builder(http::status::forbidden);
}

inline auto response::not_found() -> response_builder
{
  return response_builder(http::status::not_found);
}

inline auto response::method_not_allowed() -> response_builder
{
  return response_builder(http::status::method_not_allowed);
}

inline auto response::not_acceptable() -> response_builder
{
  return response_builder(http::status::not_acceptable);
}

inline auto response::proxy_authentication_required() -> response_builder
{
  return response_builder(http::status::proxy_authentication_required);
}

inline auto response::request_timeout() -> response_builder
{
  return response_builder(http::status::request_timeout);
}

inline auto response::conflict() -> response_builder
{
  return response_builder(http::status::conflict);
}

inline auto response::gone() -> response_builder
{
  return response_builder(http::status::gone);
}

inline auto response::length_required() -> response_builder
{
  return response_builder(http::status::length_required);
}

inline auto response::precondition_failed() -> response_builder
{
  return response_builder(http::status::precondition_failed);
}

inline auto response::payload_too_large() -> response_builder
{
  return response_builder(http::status::payload_too_large);
}

inline auto response::uri_too_long() -> response_builder
{
  return response_builder(http::status::uri_too_long);
}

inline auto response::unsupported_media_type() -> response_builder
{
  return response_builder(http::status::unsupported_media_type);
}

inline auto response::range_not_satisfiable() -> response_builder
{
  return response_builder(http::status::range_not_satisfiable);
}

inline auto response::expectation_failed() -> response_builder
{
  return response_builder(http::status::expectation_failed);
}

inline auto response::misdirected_request() -> response_builder
{
  return response_builder(http::status::misdirected_request);
}

inline auto response::unprocessable_entity() -> response_builder
{
  return response_builder(http::status::unprocessable_entity);
}

inline auto response::locked() -> response_builder
{
  return response_builder(http::status::locked);
}

inline auto response::failed_dependency() -> response_builder
{
  return response_builder(http::status::failed_dependency);
}

inline auto response::upgrade_required() -> response_builder
{
  return response_builder(http::status::upgrade_required);
}

inline auto response::precondition_required() -> response_builder
{
  return response_builder(http::status::precondition_required);
}

inline auto response::too_many_requests() -> response_builder
{
  return response_builder(http::status::too_many_requests);
}

inline auto response::request_header_fields_too_large() -> response_builder
{
  return response_builder(http::status::request_header_fields_too_large);
}

inline auto response::connection_closed_without_response() -> response_builder
{
  return response_builder(http::status::connection_closed_without_response);
}

inline auto response::unavailable_for_legal_reasons() -> response_builder
{
  return response_builder(http::status::unavailable_for_legal_reasons);
}

inline auto response::client_closed_request() -> response_builder
{
  return response_builder(http::status::client_closed_request);
}

inline auto response::internal_server_error() -> response_builder
{
  return response_builder(http::status::internal_server_error);
}

inline auto response::not_implemented() -> response_builder
{
  return response_builder(http::status::not_implemented);
}

inline auto response::bad_gateway() -> response_builder
{
  return response_builder(http::status::bad_gateway);
}

inline auto response::service_unavailable() -> response_builder
{
  return response_builder(http::status::service_unavailable);
}

inline auto response::gateway_timeout() -> response_builder
{
  return response_builder(http::status::gateway_timeout);
}

inline auto response::http_version_not_supported() -> response_builder
{
  return response_builder(http::status::http_version_not_supported);
}

inline auto response::variant_also_negotiates() -> response_builder
{
  return response_builder(http::status::variant_also_negotiates);
}

inline auto response::insufficient_storage() -> response_builder
{
  return response_builder(http::status::insufficient_storage);
}

inline auto response::loop_detected() -> response_builder
{
  return response_builder(http::status::loop_detected);
}

inline auto response::not_extended() -> response_builder
{
  return response_builder(http::status::not_extended);
}

inline auto response::network_authentication_required() -> response_builder
{
  return response_builder(http::status::network_authentication_required);
}

inline auto response::network_connect_timeout_error() -> response_builder
{
  return response_builder(http::status::network_connect_timeout_error);
}

}

FITORIA_NAMESPACE_END

#endif
