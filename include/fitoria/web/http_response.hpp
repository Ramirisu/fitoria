//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_WEB_HTTP_RESPONSE_HPP
#define FITORIA_WEB_HTTP_RESPONSE_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/core/json.hpp>

#include <fitoria/web/any_async_readable_stream.hpp>
#include <fitoria/web/async_readable_vector_stream.hpp>
#include <fitoria/web/http.hpp>
#include <fitoria/web/http_fields.hpp>

#include <span>

FITORIA_NAMESPACE_BEGIN

namespace web {

class http_response_builder;

class http_response {
  friend class http_response_builder;

  http::status_code status_code_ = http::status::ok;
  http_fields fields_;
  optional<any_async_readable_stream> body_;

  http_response(http::status_code status_code,
                http_fields fields,
                optional<any_async_readable_stream> body)
      : status_code_(status_code)
      , fields_(std::move(fields))
      , body_(std::move(body))
  {
  }

public:
  http_response() = default;

  http_response(const http_response&) = delete;

  http_response& operator=(const http_response&) = delete;

  http_response(http_response&&) = default;

  http_response& operator=(http_response&&) = default;

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

  auto body() noexcept -> optional<any_async_readable_stream&>
  {
    return optional<any_async_readable_stream&>(body_);
  }

  auto body() const noexcept -> optional<const any_async_readable_stream&>
  {
    return optional<const any_async_readable_stream&>(body_);
  }

  auto builder() -> http_response_builder;

  static auto continue_() -> http_response_builder;
  static auto switching_protocols() -> http_response_builder;
  static auto processing() -> http_response_builder;
  static auto ok() -> http_response_builder;
  static auto created() -> http_response_builder;
  static auto accepted() -> http_response_builder;
  static auto non_authoritative_information() -> http_response_builder;
  static auto no_content() -> http_response_builder;
  static auto reset_content() -> http_response_builder;
  static auto partial_content() -> http_response_builder;
  static auto multi_status() -> http_response_builder;
  static auto already_reported() -> http_response_builder;
  static auto im_used() -> http_response_builder;
  static auto multiple_choices() -> http_response_builder;
  static auto moved_permanently() -> http_response_builder;
  static auto found() -> http_response_builder;
  static auto see_other() -> http_response_builder;
  static auto not_modified() -> http_response_builder;
  static auto use_proxy() -> http_response_builder;
  static auto temporary_redirect() -> http_response_builder;
  static auto permanent_redirect() -> http_response_builder;
  static auto bad_request() -> http_response_builder;
  static auto unauthorized() -> http_response_builder;
  static auto payment_required() -> http_response_builder;
  static auto forbidden() -> http_response_builder;
  static auto not_found() -> http_response_builder;
  static auto method_not_allowed() -> http_response_builder;
  static auto not_acceptable() -> http_response_builder;
  static auto proxy_authentication_required() -> http_response_builder;
  static auto request_timeout() -> http_response_builder;
  static auto conflict() -> http_response_builder;
  static auto gone() -> http_response_builder;
  static auto length_required() -> http_response_builder;
  static auto precondition_failed() -> http_response_builder;
  static auto payload_too_large() -> http_response_builder;
  static auto uri_too_long() -> http_response_builder;
  static auto unsupported_media_type() -> http_response_builder;
  static auto range_not_satisfiable() -> http_response_builder;
  static auto expectation_failed() -> http_response_builder;
  static auto misdirected_request() -> http_response_builder;
  static auto unprocessable_entity() -> http_response_builder;
  static auto locked() -> http_response_builder;
  static auto failed_dependency() -> http_response_builder;
  static auto upgrade_required() -> http_response_builder;
  static auto precondition_required() -> http_response_builder;
  static auto too_many_requests() -> http_response_builder;
  static auto request_header_fields_too_large() -> http_response_builder;
  static auto connection_closed_without_response() -> http_response_builder;
  static auto unavailable_for_legal_reasons() -> http_response_builder;
  static auto client_closed_request() -> http_response_builder;
  static auto internal_server_error() -> http_response_builder;
  static auto not_implemented() -> http_response_builder;
  static auto bad_gateway() -> http_response_builder;
  static auto service_unavailable() -> http_response_builder;
  static auto gateway_timeout() -> http_response_builder;
  static auto http_version_not_supported() -> http_response_builder;
  static auto variant_also_negotiates() -> http_response_builder;
  static auto insufficient_storage() -> http_response_builder;
  static auto loop_detected() -> http_response_builder;
  static auto not_extended() -> http_response_builder;
  static auto network_authentication_required() -> http_response_builder;
  static auto network_connect_timeout_error() -> http_response_builder;
};

class http_response_builder {
  friend class http_response;

  http::status_code status_code_ = http::status::ok;
  http_fields fields_;
  optional<any_async_readable_stream> body_;

  http_response_builder(http::status_code status_code,
                        http_fields fields,
                        optional<any_async_readable_stream> body)
      : status_code_(status_code)
      , fields_(std::move(fields))
      , body_(std::move(body))
  {
  }

public:
  explicit http_response_builder(http::status status)
      : status_code_(status)
  {
  }

  http_response_builder(const http_response_builder&) = delete;

  http_response_builder& operator=(const http_response_builder&) = delete;

  http_response_builder(http_response_builder&&) = default;

  http_response_builder& operator=(http_response_builder&&) = default;

  auto set_status_code(http::status_code status_code) & noexcept
      -> http_response_builder&
  {
    status_code_ = status_code;
    return *this;
  }

  auto set_status_code(http::status_code status_code) && noexcept
      -> http_response_builder&&
  {
    status_code_ = status_code;
    return std::move(*this);
  }

  auto set_field(std::string_view name,
                 std::string_view value) & -> http_response_builder&
  {
    fields_.set(name, value);
    return *this;
  }

  auto set_field(std::string_view name,
                 std::string_view value) && -> http_response_builder&&
  {
    fields_.set(name, value);
    return std::move(*this);
  }

  auto set_field(http::field name,
                 std::string_view value) & -> http_response_builder&
  {
    fields_.set(name, value);
    return *this;
  }

  auto set_field(http::field name,
                 std::string_view value) && -> http_response_builder&&
  {
    fields_.set(name, value);
    return std::move(*this);
  }

  auto insert_field(std::string_view name,
                    std::string_view value) & -> http_response_builder&
  {
    fields_.insert(name, value);
    return *this;
  }

  auto insert_field(std::string_view name,
                    std::string_view value) && -> http_response_builder&&
  {
    fields_.insert(name, value);
    return std::move(*this);
  }

  auto insert_field(http::field name,
                    std::string_view value) & -> http_response_builder&
  {
    fields_.insert(name, value);
    return *this;
  }

  auto insert_field(http::field name,
                    std::string_view value) && -> http_response_builder&&
  {
    fields_.insert(name, value);
    return std::move(*this);
  }

  auto build() -> http_response
  {
    return http_response(status_code_, std::move(fields_), std::move(body_));
  }

  template <std::size_t N>
  auto set_body(std::span<const std::byte, N> bytes) -> http_response
  {
    body_.emplace(async_readable_vector_stream(bytes));
    return build();
  }

  auto set_body(std::string_view sv) -> http_response
  {
    return set_body(std::as_bytes(std::span(sv.begin(), sv.end())));
  }

  auto set_json(const boost::json::value& jv) -> http_response
  {
    set_field(http::field::content_type, http::fields::content_type::json());
    auto str = boost::json::serialize(jv);
    return set_body(std::as_bytes(std::span(str.begin(), str.end())));
  }

  template <typename T>
    requires(boost::json::has_value_from<T>::value)
  auto set_json(const T& obj) -> http_response
  {
    return set_json(boost::json::value_from(obj));
  }

  template <async_readable_stream AsyncReadableStream>
  auto set_stream(AsyncReadableStream&& stream) -> http_response
  {
    body_.emplace(std::forward<AsyncReadableStream>(stream));
    return build();
  }
};

inline auto http_response::builder() -> http_response_builder
{
  return http_response_builder(
      status_code_, std::move(fields_), std::move(body_));
}

inline auto http_response::continue_() -> http_response_builder
{
  return http_response_builder(http::status::continue_);
}

inline auto http_response::switching_protocols() -> http_response_builder
{
  return http_response_builder(http::status::switching_protocols);
}

inline auto http_response::processing() -> http_response_builder
{
  return http_response_builder(http::status::processing);
}

inline auto http_response::ok() -> http_response_builder
{
  return http_response_builder(http::status::ok);
}

inline auto http_response::created() -> http_response_builder
{
  return http_response_builder(http::status::created);
}

inline auto http_response::accepted() -> http_response_builder
{
  return http_response_builder(http::status::accepted);
}

inline auto http_response::non_authoritative_information()
    -> http_response_builder
{
  return http_response_builder(http::status::non_authoritative_information);
}

inline auto http_response::no_content() -> http_response_builder
{
  return http_response_builder(http::status::no_content);
}

inline auto http_response::reset_content() -> http_response_builder
{
  return http_response_builder(http::status::reset_content);
}

inline auto http_response::partial_content() -> http_response_builder
{
  return http_response_builder(http::status::partial_content);
}

inline auto http_response::multi_status() -> http_response_builder
{
  return http_response_builder(http::status::multi_status);
}

inline auto http_response::already_reported() -> http_response_builder
{
  return http_response_builder(http::status::already_reported);
}

inline auto http_response::im_used() -> http_response_builder
{
  return http_response_builder(http::status::im_used);
}

inline auto http_response::multiple_choices() -> http_response_builder
{
  return http_response_builder(http::status::multiple_choices);
}

inline auto http_response::moved_permanently() -> http_response_builder
{
  return http_response_builder(http::status::moved_permanently);
}

inline auto http_response::found() -> http_response_builder
{
  return http_response_builder(http::status::found);
}

inline auto http_response::see_other() -> http_response_builder
{
  return http_response_builder(http::status::see_other);
}

inline auto http_response::not_modified() -> http_response_builder
{
  return http_response_builder(http::status::not_modified);
}

inline auto http_response::use_proxy() -> http_response_builder
{
  return http_response_builder(http::status::use_proxy);
}

inline auto http_response::temporary_redirect() -> http_response_builder
{
  return http_response_builder(http::status::temporary_redirect);
}

inline auto http_response::permanent_redirect() -> http_response_builder
{
  return http_response_builder(http::status::permanent_redirect);
}

inline auto http_response::bad_request() -> http_response_builder
{
  return http_response_builder(http::status::bad_request);
}

inline auto http_response::unauthorized() -> http_response_builder
{
  return http_response_builder(http::status::unauthorized);
}

inline auto http_response::payment_required() -> http_response_builder
{
  return http_response_builder(http::status::payment_required);
}

inline auto http_response::forbidden() -> http_response_builder
{
  return http_response_builder(http::status::forbidden);
}

inline auto http_response::not_found() -> http_response_builder
{
  return http_response_builder(http::status::not_found);
}

inline auto http_response::method_not_allowed() -> http_response_builder
{
  return http_response_builder(http::status::method_not_allowed);
}

inline auto http_response::not_acceptable() -> http_response_builder
{
  return http_response_builder(http::status::not_acceptable);
}

inline auto http_response::proxy_authentication_required()
    -> http_response_builder
{
  return http_response_builder(http::status::proxy_authentication_required);
}

inline auto http_response::request_timeout() -> http_response_builder
{
  return http_response_builder(http::status::request_timeout);
}

inline auto http_response::conflict() -> http_response_builder
{
  return http_response_builder(http::status::conflict);
}

inline auto http_response::gone() -> http_response_builder
{
  return http_response_builder(http::status::gone);
}

inline auto http_response::length_required() -> http_response_builder
{
  return http_response_builder(http::status::length_required);
}

inline auto http_response::precondition_failed() -> http_response_builder
{
  return http_response_builder(http::status::precondition_failed);
}

inline auto http_response::payload_too_large() -> http_response_builder
{
  return http_response_builder(http::status::payload_too_large);
}

inline auto http_response::uri_too_long() -> http_response_builder
{
  return http_response_builder(http::status::uri_too_long);
}

inline auto http_response::unsupported_media_type() -> http_response_builder
{
  return http_response_builder(http::status::unsupported_media_type);
}

inline auto http_response::range_not_satisfiable() -> http_response_builder
{
  return http_response_builder(http::status::range_not_satisfiable);
}

inline auto http_response::expectation_failed() -> http_response_builder
{
  return http_response_builder(http::status::expectation_failed);
}

inline auto http_response::misdirected_request() -> http_response_builder
{
  return http_response_builder(http::status::misdirected_request);
}

inline auto http_response::unprocessable_entity() -> http_response_builder
{
  return http_response_builder(http::status::unprocessable_entity);
}

inline auto http_response::locked() -> http_response_builder
{
  return http_response_builder(http::status::locked);
}

inline auto http_response::failed_dependency() -> http_response_builder
{
  return http_response_builder(http::status::failed_dependency);
}

inline auto http_response::upgrade_required() -> http_response_builder
{
  return http_response_builder(http::status::upgrade_required);
}

inline auto http_response::precondition_required() -> http_response_builder
{
  return http_response_builder(http::status::precondition_required);
}

inline auto http_response::too_many_requests() -> http_response_builder
{
  return http_response_builder(http::status::too_many_requests);
}

inline auto http_response::request_header_fields_too_large()
    -> http_response_builder
{
  return http_response_builder(http::status::request_header_fields_too_large);
}

inline auto http_response::connection_closed_without_response()
    -> http_response_builder
{
  return http_response_builder(
      http::status::connection_closed_without_response);
}

inline auto http_response::unavailable_for_legal_reasons()
    -> http_response_builder
{
  return http_response_builder(http::status::unavailable_for_legal_reasons);
}

inline auto http_response::client_closed_request() -> http_response_builder
{
  return http_response_builder(http::status::client_closed_request);
}

inline auto http_response::internal_server_error() -> http_response_builder
{
  return http_response_builder(http::status::internal_server_error);
}

inline auto http_response::not_implemented() -> http_response_builder
{
  return http_response_builder(http::status::not_implemented);
}

inline auto http_response::bad_gateway() -> http_response_builder
{
  return http_response_builder(http::status::bad_gateway);
}

inline auto http_response::service_unavailable() -> http_response_builder
{
  return http_response_builder(http::status::service_unavailable);
}

inline auto http_response::gateway_timeout() -> http_response_builder
{
  return http_response_builder(http::status::gateway_timeout);
}

inline auto http_response::http_version_not_supported() -> http_response_builder
{
  return http_response_builder(http::status::http_version_not_supported);
}

inline auto http_response::variant_also_negotiates() -> http_response_builder
{
  return http_response_builder(http::status::variant_also_negotiates);
}

inline auto http_response::insufficient_storage() -> http_response_builder
{
  return http_response_builder(http::status::insufficient_storage);
}

inline auto http_response::loop_detected() -> http_response_builder
{
  return http_response_builder(http::status::loop_detected);
}

inline auto http_response::not_extended() -> http_response_builder
{
  return http_response_builder(http::status::not_extended);
}

inline auto http_response::network_authentication_required()
    -> http_response_builder
{
  return http_response_builder(http::status::network_authentication_required);
}

inline auto http_response::network_connect_timeout_error()
    -> http_response_builder
{
  return http_response_builder(http::status::network_connect_timeout_error);
}

}

FITORIA_NAMESPACE_END

#endif
