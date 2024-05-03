//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_CORE_EXPECTED_HPP
#define FITORIA_CORE_EXPECTED_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/core/format.hpp>
#include <fitoria/core/type_traits.hpp>

#include <functional>
#include <initializer_list>
#include <memory>
#include <utility>

FITORIA_NAMESPACE_BEGIN

template <typename E>
class unexpected {
public:
  static_assert(std::is_same_v<std::remove_cvref_t<E>, E>);
  static_assert(!is_specialization_of_v<std::remove_cvref_t<E>, unexpected>);
  static_assert(std::is_object_v<E>);
  static_assert(std::is_destructible_v<std::remove_cvref_t<E>>);
  static_assert(!std::is_array_v<std::remove_cvref_t<E>>);

  template <typename E2 = E>
  constexpr explicit unexpected(E2&& err)
    requires(std::is_constructible_v<E, E2>
             && !std::is_same_v<std::remove_cvref_t<E2>, unexpected>
             && !std::is_same_v<std::remove_cvref_t<E2>, std::in_place_t>)
      : err_(std::forward<E2>(err))
  {
  }

  template <typename... Args>
  constexpr explicit unexpected(std::in_place_t, Args&&... args)
    requires(std::is_constructible_v<E, Args...>)
      : err_(std::forward<Args>(args)...)
  {
  }

  template <typename U, typename... Args>
  constexpr explicit unexpected(std::in_place_t,
                                std::initializer_list<U> ilist,
                                Args&&... args)
    requires(std::is_constructible_v<E, std::initializer_list<U>&, Args...>)
      : err_(ilist, std::forward<Args>(args)...)
  {
  }

  constexpr unexpected(const unexpected&) = default;

  constexpr unexpected(unexpected&&) = default;

  constexpr E& error() & noexcept
  {
    return err_;
  }

  constexpr const E& error() const& noexcept
  {
    return err_;
  }

  constexpr E&& error() && noexcept
  {
    return std::move(err_);
  }

  constexpr const E&& error() const&& noexcept
  {
    return std::move(err_);
  }

  constexpr void
  swap(unexpected& other) noexcept(std::is_nothrow_swappable_v<E>)
    requires(std::is_swappable_v<E>)
  {
    using std::swap;
    swap(error(), other.error());
  }

  template <typename E2>
  friend constexpr bool operator==(const unexpected& x, const unexpected<E2>& y)
  {
    return x.error() == y.error();
  }

  friend constexpr void swap(unexpected& x,
                             unexpected& y) noexcept(noexcept(x.swap(y)))
    requires(std::is_swappable_v<E>)
  {
    x.swap(y);
  }

private:
  E err_;
};

template <typename E>
unexpected(E) -> unexpected<E>;

#if !FITORIA_NO_EXCEPTIONS

template <typename E>
class bad_expected_access;

template <>
class bad_expected_access<void> : public std::exception {
  const char* what() const noexcept override
  {
    return "bad expected access";
  }

protected:
  bad_expected_access() = default;

  bad_expected_access(const bad_expected_access&) = default;

  bad_expected_access(bad_expected_access&&) = default;

  bad_expected_access& operator=(const bad_expected_access&) = default;

  bad_expected_access& operator=(bad_expected_access&&) = default;

  ~bad_expected_access() override = default;
};

template <typename E>
class bad_expected_access : public bad_expected_access<void> {
public:
  constexpr explicit bad_expected_access(E err)
      : err_(std::move(err))
  {
  }

  bad_expected_access(const bad_expected_access&) = default;

  bad_expected_access(bad_expected_access&&) = default;

  bad_expected_access& operator=(const bad_expected_access&) = default;

  bad_expected_access& operator=(bad_expected_access&&) = default;

  ~bad_expected_access() override = default;

  const char* what() const noexcept override
  {
    return "bad expected access";
  }

  constexpr E& error() & noexcept
  {
    return err_;
  }

  constexpr const E& error() const& noexcept
  {
    return err_;
  }

  constexpr E&& error() && noexcept
  {
    return std::move(err_);
  }

  constexpr const E&& error() const&& noexcept
  {
    return std::move(err_);
  }

private:
  E err_;
};

#endif
struct unexpect_t {
  struct tag_t {
    constexpr explicit tag_t() noexcept = default;
  };

  constexpr explicit unexpect_t(tag_t) noexcept { }
};

inline constexpr unexpect_t unexpect { unexpect_t::tag_t {} };

template <typename T, typename E>
class expected {
  template <typename U, typename G>
  static constexpr bool is_expected_like_constructible_v
      = std::is_constructible_v<T, expected<U, G>&>
      || std::is_constructible_v<T, expected<U, G>>
      || std::is_constructible_v<T, const expected<U, G>&>
      || std::is_constructible_v<T, const expected<U, G>>
      || std::is_convertible_v<expected<U, G>&, T>
      || std::is_convertible_v<expected<U, G>, T>
      || std::is_convertible_v<const expected<U, G>&, T>
      || std::is_convertible_v<const expected<U, G>, T>
      || std::is_constructible_v<unexpected<E>, expected<U, G>&>
      || std::is_constructible_v<unexpected<E>, expected<U, G>>
      || std::is_constructible_v<unexpected<E>, const expected<U, G>&>
      || std::is_constructible_v<unexpected<E>, const expected<U, G>>;

  template <typename NewType, typename OldType, typename... Args>
  constexpr void
  reinit_expected(NewType& new_val, OldType& old_val, Args&&... args)
  {
    if constexpr (std::is_nothrow_constructible_v<NewType, Args...>) {
      std::destroy_at(std::addressof(old_val));
      std::construct_at(std::addressof(new_val), std::forward<Args>(args)...);
    } else if constexpr (std::is_nothrow_move_constructible_v<NewType>) {
      NewType temp(std::forward<Args>(args)...);
      std::destroy_at(std::addressof(old_val));
      std::construct_at(std::addressof(new_val), std::move(temp));
    } else {
      OldType temp(std::move(old_val));
      std::destroy_at(std::addressof(old_val));
      try {
        std::construct_at(std::addressof(new_val), std::forward<Args>(args)...);
      } catch (...) {
        std::construct_at(std::addressof(old_val), std::move(temp));
        throw;
      }
    }
  }

public:
  static_assert(!std::is_same_v<std::remove_cvref_t<T>, std::in_place_t>);
  static_assert(!std::is_same_v<std::remove_cvref_t<T>, unexpect_t>);
  static_assert(!is_specialization_of_v<std::remove_cvref_t<T>, unexpected>);
  static_assert(!std::is_array_v<std::remove_cvref_t<T>>);
  static_assert(!std::is_reference_v<std::remove_cvref_t<T>>);
  static_assert(std::is_destructible_v<std::remove_cvref_t<T>>);

  static_assert(std::is_same_v<std::remove_cvref_t<E>, E>);
  static_assert(!is_specialization_of_v<std::remove_cvref_t<E>, unexpected>);
  static_assert(std::is_object_v<E>);
  static_assert(std::is_destructible_v<std::remove_cvref_t<E>>);
  static_assert(!std::is_array_v<std::remove_cvref_t<E>>);

  using value_type = T;
  using error_type = E;
  using unexpected_type = unexpected<E>;

  template <typename U>
  using rebind = expected<U, E>;

  constexpr expected()
    requires(std::is_default_constructible_v<T>)
      : val_()
      , has_(true)
  {
  }

  constexpr expected(const expected& other)
    requires(!(std::is_trivially_copy_constructible_v<T>
               && std::is_trivially_copy_constructible_v<E>)
             && std::is_copy_constructible_v<T>
             && std::is_copy_constructible_v<E>)
  {
    if (other) {
      std::construct_at(std::addressof(this->val_), other.value());
    } else {
      std::construct_at(std::addressof(this->err_), other.error());
    }
    this->has_ = other.has_value();
  }

  constexpr expected(const expected&)
    requires(std::is_trivially_copy_constructible_v<T>
             && std::is_trivially_copy_constructible_v<E>)
  = default;

  constexpr expected(expected&& other) noexcept(
      std::is_nothrow_move_constructible_v<T>
      && std::is_nothrow_move_constructible_v<E>)
    requires(!(std::is_trivially_move_constructible_v<T>
               && std::is_trivially_move_constructible_v<E>)
             && std::is_move_constructible_v<T>
             && std::is_move_constructible_v<E>)
  {
    if (other) {
      std::construct_at(std::addressof(this->val_), std::move(other.value()));
    } else {
      std::construct_at(std::addressof(this->err_), std::move(other.error()));
    }
    this->has_ = other.has_value();
  }

  constexpr expected(expected&&)
    requires(std::is_trivially_move_constructible_v<T>
             && std::is_trivially_move_constructible_v<E>)
  = default;

  template <typename U, typename G>
  constexpr explicit(std::is_convertible_v<const U&, T>
                     || std::is_convertible_v<const G&, E>)
      expected(const expected<U, G>& other)
    requires(std::is_constructible_v<T, const U&>
             && std::is_constructible_v<E, const G&>
             && !is_expected_like_constructible_v<U, G>)
  {
    if (other) {
      std::construct_at(std::addressof(this->val_), other.value());
    } else {
      std::construct_at(std::addressof(this->err_), other.error());
    }
    this->has_ = other.has_value();
  }

  template <typename U, typename G>
  constexpr explicit(std::is_convertible_v<U, T> || std::is_convertible_v<G, E>)
      expected(expected<U, G>&& other)
    requires(std::is_constructible_v<T, U> && std::is_constructible_v<E, G>
             && !is_expected_like_constructible_v<U, G>)
  {
    if (other) {
      std::construct_at(std::addressof(this->val_), std::move(other.value()));
    } else {
      std::construct_at(std::addressof(this->err_), std::move(other.error()));
    }
    this->has_ = other.has_value();
  }

  template <typename U = T>
  constexpr explicit(!std::is_convertible_v<U, T>) expected(U&& value)
    requires(std::is_constructible_v<T, U>
             && !std::is_same_v<std::remove_cvref_t<U>, std::in_place_t>
             && !std::is_same_v<std::remove_cvref_t<U>, expected>
             && !is_specialization_of_v<std::remove_cvref_t<U>, unexpected>)
  {
    std::construct_at(std::addressof(this->val_), std::forward<U>(value));
    this->has_ = true;
  }

  template <typename G>
  constexpr explicit(!std::is_convertible_v<const G&, E>)
      expected(const unexpected<G>& other)
  {
    std::construct_at(std::addressof(this->err_), other.error());
    this->has_ = false;
  }

  template <typename G>
  constexpr explicit(!std::is_convertible_v<G, E>)
      expected(unexpected<G>&& other)
  {
    std::construct_at(std::addressof(this->err_), std::move(other.error()));
    this->has_ = false;
  }

  template <typename... Args>
  constexpr explicit expected(std::in_place_t, Args&&... args)
    requires(std::is_constructible_v<T, Args...>)
  {
    std::construct_at(std::addressof(this->val_), std::forward<Args>(args)...);
    this->has_ = true;
  }

  template <typename U, typename... Args>
  constexpr explicit expected(std::in_place_t,
                              std::initializer_list<U> ilist,
                              Args&&... args)
    requires(std::is_constructible_v<T, std::initializer_list<U>&, Args...>)
  {
    std::construct_at(
        std::addressof(this->val_), ilist, std::forward<Args>(args)...);
    this->has_ = true;
  }

  template <typename... Args>
  constexpr explicit expected(unexpect_t, Args&&... args)
    requires(std::is_constructible_v<E, Args...>)
  {
    std::construct_at(std::addressof(this->err_), std::forward<Args>(args)...);
    this->has_ = false;
  }

  template <typename U, typename... Args>
  constexpr explicit expected(unexpect_t,
                              std::initializer_list<U> ilist,
                              Args&&... args)
    requires(std::is_constructible_v<E, std::initializer_list<U>&, Args...>)
  {
    std::construct_at(
        std::addressof(this->err_), ilist, std::forward<Args>(args)...);
    this->has_ = false;
  }

  constexpr ~expected()
    requires(!std::is_trivially_destructible_v<T>
             && !std::is_trivially_destructible_v<E>)
  {
    if (has_value()) {
      std::destroy_at(std::addressof(this->val_));
    } else {
      std::destroy_at(std::addressof(this->err_));
    }
  }

  constexpr ~expected()
    requires(!std::is_trivially_destructible_v<T>
             && std::is_trivially_destructible_v<E>)
  {
    if (has_value()) {
      std::destroy_at(std::addressof(this->val_));
    }
  }

  constexpr ~expected()
    requires(std::is_trivially_destructible_v<T>
             && !std::is_trivially_destructible_v<E>)
  {
    if (!has_value()) {
      std::destroy_at(std::addressof(this->err_));
    }
  }

  constexpr ~expected()
    requires(std::is_trivially_destructible_v<T>
             && std::is_trivially_destructible_v<E>)
  = default;

  constexpr expected& operator=(const expected& other)
    requires(!(std::is_trivially_copy_assignable_v<T>
               && std::is_trivially_copy_assignable_v<E>)
             && std::is_copy_constructible_v<T> && std::is_copy_assignable_v<T>
             && std::is_copy_constructible_v<E> && std::is_copy_assignable_v<E>
             && (std::is_nothrow_move_constructible_v<T>
                 || std::is_nothrow_move_constructible_v<E>))
  {
    if (has_value()) {
      if (other) {
        this->val_ = other.value();
      } else {
        reinit_expected(this->err_, this->val_, other.error());
        this->has_ = false;
      }
    } else {
      if (other) {
        reinit_expected(this->val_, this->err_, other.value());
        this->has_ = true;
      } else {
        this->err_ = other.error();
      }
    }

    return *this;
  }

  constexpr expected& operator=(const expected&)
    requires(std::is_trivially_copy_assignable_v<T>
             && std::is_trivially_copy_assignable_v<E>)
  = default;

  constexpr expected& operator=(expected&& other) noexcept(
      std::is_nothrow_move_constructible_v<T>
      && std::is_nothrow_move_assignable_v<T>
      && std::is_nothrow_move_constructible_v<E>
      && std::is_nothrow_move_assignable_v<E>)
    requires(!(std::is_trivially_move_assignable_v<T>
               && std::is_trivially_move_assignable_v<E>)
             && std::is_move_constructible_v<T> && std::is_move_assignable_v<T>
             && std::is_move_constructible_v<E> && std::is_move_assignable_v<E>
             && (std::is_nothrow_move_constructible_v<T>
                 || std::is_nothrow_move_constructible_v<E>))
  {
    if (has_value()) {
      if (other) {
        this->val_ = std::move(other.value());
      } else {
        reinit_expected(this->err_, this->val_, std::move(other.error()));
        this->has_ = false;
      }
    } else {
      if (other) {
        reinit_expected(this->val_, this->err_, std::move(other.value()));
        this->has_ = true;
      } else {
        this->err_ = std::move(other.error());
      }
    }

    return *this;
  }

  constexpr expected& operator=(expected&&)
    requires(std::is_trivially_move_assignable_v<T>
             && std::is_trivially_move_assignable_v<E>)
  = default;

  template <typename U = T>
  constexpr expected& operator=(U&& value)
    requires(
        std::is_constructible_v<T, U> && std::is_assignable_v<T&, U>
        && !std::is_same_v<std::remove_cvref_t<U>, expected>
        && !is_specialization_of_v<std::remove_cvref_t<U>, fitoria::expected>
        && (std::is_nothrow_constructible_v<T, U>
            || std::is_nothrow_move_constructible_v<T>
            || std::is_nothrow_move_constructible_v<E>))
  {
    if (has_value()) {
      this->val_ = std::forward<U>(value);
    } else {
      reinit_expected(this->val_, this->err_, std::forward<U>(value));
      this->has_ = true;
    }

    return *this;
  }

  template <typename G>
  constexpr expected& operator=(const unexpected<G>& other)
    requires(std::is_constructible_v<E, const G&>
             && std::is_assignable_v<E&, const G&>
             && (std::is_nothrow_constructible_v<E, const G&>
                 || std::is_nothrow_move_constructible_v<T>
                 || std::is_nothrow_move_constructible_v<E>))
  {
    if (has_value()) {
      reinit_expected(this->err_, this->val_, other.error());
      this->has_ = false;
    } else {
      this->err_ = other.error();
    }

    return *this;
  }

  template <typename G>
  constexpr expected& operator=(unexpected<G>&& other)
    requires(std::is_constructible_v<E, G> && std::is_assignable_v<E&, G>
             && (std::is_nothrow_constructible_v<E, G>
                 || std::is_nothrow_move_constructible_v<T>
                 || std::is_nothrow_move_constructible_v<E>))
  {
    if (has_value()) {
      reinit_expected(this->err_, this->val_, std::move(other.error()));
      this->has_ = false;
    } else {
      this->err_ = std::move(other.error());
    }

    return *this;
  }

  constexpr std::add_pointer_t<T> operator->() noexcept
  {
    FITORIA_ASSERT(has_value());
    return std::addressof(this->val_);
  }

  constexpr std::add_pointer_t<const T> operator->() const noexcept
  {
    FITORIA_ASSERT(has_value());
    return std::addressof(this->val_);
  }

  constexpr T& operator*() & noexcept
  {
    FITORIA_ASSERT(has_value());
    return this->val_;
  }

  constexpr const T& operator*() const& noexcept
  {
    FITORIA_ASSERT(has_value());
    return this->val_;
  }

  constexpr T&& operator*() && noexcept
  {
    FITORIA_ASSERT(has_value());
    return std::move(this->val_);
  }

  constexpr const T&& operator*() const&& noexcept
  {
    FITORIA_ASSERT(has_value());
    return std::move(this->val_);
  }

  constexpr explicit operator bool() const noexcept
  {
    return has_value();
  }

  constexpr bool has_value() const noexcept
  {
    return this->has_;
  }

  constexpr T& value() &
  {
    if (!has_value()) {
      FITORIA_THROW_OR(bad_expected_access(error()), std::terminate());
    }

    return this->val_;
  }

  constexpr const T& value() const&
  {
    if (!has_value()) {
      FITORIA_THROW_OR(bad_expected_access(error()), std::terminate());
    }

    return this->val_;
  }

  constexpr T&& value() &&
  {
    if (!has_value()) {
      FITORIA_THROW_OR(bad_expected_access(std::move(error())),
                       std::terminate());
    }

    return std::move(this->val_);
  }

  constexpr const T&& value() const&&
  {
    if (!has_value()) {
      FITORIA_THROW_OR(bad_expected_access(std::move(error())),
                       std::terminate());
    }

    return std::move(this->val_);
  }

  constexpr E& error() & noexcept
  {
    FITORIA_ASSERT(!has_value());
    return this->err_;
  }

  constexpr const E& error() const& noexcept
  {
    FITORIA_ASSERT(!has_value());
    return this->err_;
  }

  constexpr E&& error() && noexcept
  {
    FITORIA_ASSERT(!has_value());
    return std::move(this->err_);
  }

  constexpr const E&& error() const&& noexcept
  {
    FITORIA_ASSERT(!has_value());
    return std::move(this->err_);
  }

  template <typename U>
  constexpr T value_or(U&& default_value) const&
    requires(std::is_copy_constructible_v<T> && std::is_convertible_v<U, T>)
  {
    if (has_value()) {
      return value();
    } else {
      return static_cast<T>(std::forward<U>(default_value));
    }
  }

  template <typename U>
  constexpr T value_or(U&& default_value) &&
    requires(std::is_move_constructible_v<T> && std::is_convertible_v<U, T>)
  {
    if (has_value()) {
      return std::move(value());
    } else {
      return static_cast<T>(std::forward<U>(default_value));
    }
  }

  template <typename G>
  constexpr E error_or(G&& default_error) const&
    requires(std::is_copy_constructible_v<E> && std::is_convertible_v<G, E>)
  {
    if (has_value()) {
      return static_cast<E>(std::forward<G>(default_error));
    } else {
      return error();
    }
  }

  template <typename G>
  constexpr E error_or(G&& default_error) &&
    requires(std::is_move_constructible_v<E> && std::is_convertible_v<G, E>)
  {
    if (has_value()) {
      return static_cast<E>(std::forward<G>(default_error));
    } else {
      return std::move(error());
    }
  }

  template <typename F> 
  constexpr auto and_then(F&& f) & 
    requires(std::is_copy_constructible_v<E>)
  {
    using U = std::remove_cvref_t<std::invoke_result_t<F, T&>>;
    static_assert(is_specialization_of_v<U, fitoria::expected>);
    static_assert(std::is_same_v<typename U::error_type, E>);
    if (has_value()) {
      return U(std::invoke(std::forward<F>(f), value()));
    } else {
      return U(unexpect, error());
    }
  }

  template <typename F>
  constexpr auto and_then(F&& f) const&
    requires(std::is_copy_constructible_v<E>)
  {
    using U = std::remove_cvref_t<std::invoke_result_t<F, const T&>>;
    static_assert(is_specialization_of_v<U, fitoria::expected>);
    static_assert(std::is_same_v<typename U::error_type, E>);
    if (has_value()) {
      return U(std::invoke(std::forward<F>(f), value()));
    } else {
      return U(unexpect, error());
    }
  }

  template <typename F>
  constexpr auto and_then(F&& f) &&
    requires(std::is_move_constructible_v<E>)
  {
    using U = std::remove_cvref_t<std::invoke_result_t<F, T>>;
    static_assert(is_specialization_of_v<U, fitoria::expected>);
    static_assert(std::is_same_v<typename U::error_type, E>);
    if (has_value()) {
      return U(std::invoke(std::forward<F>(f), std::move(value())));
    } else {
      return U(unexpect, std::move(error()));
    }
  }

  template <typename F>
  constexpr auto and_then(F&& f) const&&
    requires(std::is_move_constructible_v<E>)
  {
    using U = std::remove_cvref_t<std::invoke_result_t<F, const T>>;
    static_assert(is_specialization_of_v<U, fitoria::expected>);
    static_assert(std::is_same_v<typename U::error_type, E>);
    if (has_value()) {
      return U(std::invoke(std::forward<F>(f), std::move(value())));
    } else {
      return U(unexpect, std::move(error()));
    }
  }

  template <typename F>
  constexpr auto or_else(F&& f) &
    requires(std::is_copy_constructible_v<T>)
  {
    using G = std::remove_cvref_t<std::invoke_result_t<F, E&>>;
    static_assert(is_specialization_of_v<G, fitoria::expected>);
    static_assert(std::is_same_v<typename G::value_type, T>);
    if (has_value()) {
      return G(std::in_place, value());
    } else {
      return G(std::invoke(std::forward<F>(f), error()));
    }
  }

  template <typename F>
  constexpr auto or_else(F&& f) const&
    requires(std::is_copy_constructible_v<T>)
  {
    using G = std::remove_cvref_t<std::invoke_result_t<F, const E&>>;
    static_assert(is_specialization_of_v<G, fitoria::expected>);
    static_assert(std::is_same_v<typename G::value_type, T>);
    if (has_value()) {
      return G(std::in_place, value());
    } else {
      return G(std::invoke(std::forward<F>(f), error()));
    }
  }

  template <typename F>
  constexpr auto or_else(F&& f) &&
    requires(std::is_move_constructible_v<T>)
  {
    using G = std::remove_cvref_t<std::invoke_result_t<F, E>>;
    static_assert(is_specialization_of_v<G, fitoria::expected>);
    static_assert(std::is_same_v<typename G::value_type, T>);
    if (has_value()) {
      return G(std::in_place, std::move(value()));
    } else {
      return G(std::invoke(std::forward<F>(f), std::move(error())));
    }
  }

  template <typename F>
  constexpr auto or_else(F&& f) const&&
    requires(std::is_move_constructible_v<T>)
  {
    using G = std::remove_cvref_t<std::invoke_result_t<F, const E>>;
    static_assert(is_specialization_of_v<G, fitoria::expected>);
    static_assert(std::is_same_v<typename G::value_type, T>);
    if (has_value()) {
      return G(std::in_place, std::move(value()));
    } else {
      return G(std::invoke(std::forward<F>(f), std::move(error())));
    }
  }

  template <typename F>
  constexpr auto transform(F&& f) &
    requires(std::is_copy_constructible_v<E>)
  {
    using U = std::remove_cv_t<std::invoke_result_t<F, T&>>;
    if (has_value()) {
      return expected<U, E>(std::invoke(std::forward<F>(f), value()));
    } else {
      return expected<U, E>(unexpect, error());
    }
  }

  template <typename F>
  constexpr auto transform(F&& f) const&
    requires(std::is_copy_constructible_v<E>)
  {
    using U = std::remove_cv_t<std::invoke_result_t<F, const T&>>;
    if (has_value()) {
      return expected<U, E>(std::invoke(std::forward<F>(f), value()));
    } else {
      return expected<U, E>(unexpect, error());
    }
  }

  template <typename F>
  constexpr auto transform(F&& f) &&
    requires(std::is_move_constructible_v<E>)
  {
    using U = std::remove_cv_t<std::invoke_result_t<F, T>>;
    if (has_value()) {
      return expected<U, E>(
          std::invoke(std::forward<F>(f), std::move(value())));
    } else {
      return expected<U, E>(unexpect, std::move(error()));
    }
  }

  template <typename F>
  constexpr auto transform(F&& f) const&&
    requires(std::is_move_constructible_v<E>)
  {
    using U = std::remove_cv_t<std::invoke_result_t<F, const T>>;
    if (has_value()) {
      return expected<U, E>(
          std::invoke(std::forward<F>(f), std::move(value())));
    } else {
      return expected<U, E>(unexpect, std::move(error()));
    }
  }

  template <typename F>
  constexpr auto transform_error(F&& f) &
    requires(std::is_copy_constructible_v<T>)
  {
    using G = std::remove_cv_t<std::invoke_result_t<F, E&>>;
    if (has_value()) {
      return expected<T, G>(std::in_place, value());
    } else {
      return expected<T, G>(unexpect, std::invoke(std::forward<F>(f), error()));
    }
  }

  template <typename F>
  constexpr auto transform_error(F&& f) const&
    requires(std::is_copy_constructible_v<T>)
  {
    using G = std::remove_cv_t<std::invoke_result_t<F, const E&>>;
    if (has_value()) {
      return expected<T, G>(std::in_place, value());
    } else {
      return expected<T, G>(unexpect, std::invoke(std::forward<F>(f), error()));
    }
  }

  template <typename F>
  constexpr auto transform_error(F&& f) &&
    requires(std::is_move_constructible_v<T>)
  {
    using G = std::remove_cv_t<std::invoke_result_t<F, E>>;
    if (has_value()) {
      return expected<T, G>(std::in_place, std::move(value()));
    } else {
      return expected<T, G>(
          unexpect, std::invoke(std::forward<F>(f), std::move(error())));
    }
  }

  template <typename F>
  constexpr auto transform_error(F&& f) const&&
    requires(std::is_move_constructible_v<T>)
  {
    using G = std::remove_cv_t<std::invoke_result_t<F, const E>>;
    if (has_value()) {
      return expected<T, G>(std::in_place, std::move(value()));
    } else {
      return expected<T, G>(
          unexpect, std::invoke(std::forward<F>(f), std::move(error())));
    }
  }

  template <typename... Args>
  constexpr T& emplace(Args&&... args) noexcept
    requires(std::is_nothrow_constructible_v<T, Args...>)
  {
    if (has_value()) {
      std::destroy_at(std::addressof(this->val_));
    } else {
      std::destroy_at(std::addressof(this->err_));
    }

    std::construct_at(std::addressof(this->val_), std::forward<Args>(args)...);
    this->has_ = true;
    return this->val_;
  }

  template <typename U, typename... Args>
  constexpr T& emplace(std::initializer_list<U> ilist, Args&&... args) noexcept
    requires(
        std::is_nothrow_constructible_v<T, std::initializer_list<U>&, Args...>)
  {
    if (has_value()) {
      std::destroy_at(std::addressof(this->val_));
    } else {
      std::destroy_at(std::addressof(this->err_));
    }

    std::construct_at(
        std::addressof(this->val_), ilist, std::forward<Args>(args)...);
    this->has_ = true;
    return this->val_;
  }

  void swap(expected& other) noexcept(std::is_nothrow_move_constructible_v<T>
                                      && std::is_nothrow_swappable_v<T>
                                      && std::is_nothrow_move_constructible_v<E>
                                      && std::is_nothrow_swappable_v<E>)
    requires(std::is_swappable_v<T> && std::is_swappable_v<E>
             && std::is_move_constructible_v<T>
             && std::is_move_constructible_v<E>
             && (std::is_nothrow_move_constructible_v<T>
                 || std::is_nothrow_move_constructible_v<E>))
  {
    if (has_value()) {
      if (other.has_value()) {
        using std::swap;
        swap(value(), other.value());
      } else {
        if constexpr (std::is_nothrow_move_constructible_v<T>
                      && std::is_nothrow_move_constructible_v<E>) {
          E temp(std::move(other.err_));
          std::destroy_at(std::addressof(other.err_));
          std::construct_at(std::addressof(other.val_), std::move(this->val_));
          std::destroy_at(std::addressof(this->val_));
          std::construct_at(std::addressof(this->err_), std::move(temp));
        } else if constexpr (std::is_nothrow_move_constructible_v<E>) {
          E temp(std::move(other.err_));
          std::destroy_at(std::addressof(other.err_));
          try {
            std::construct_at(std::addressof(other.val_),
                              std::move(this->val_));
            std::destroy_at(std::addressof(this->val_));
            std::construct_at(std::addressof(this->err_), std::move(temp));
          } catch (...) {
            std::construct_at(std::addressof(other.err_), std::move(temp));
            throw;
          }
        } else {
          T temp(std::move(this->val_));
          std::destroy_at(std::addressof(this->val_));
          try {
            std::construct_at(std::addressof(this->err_),
                              std::move(other.err_));
            std::destroy_at(std::addressof(other.err_));
            std::construct_at(std::addressof(other.val_), std::move(temp));
          } catch (...) {
            std::construct_at(std::addressof(this->val_), std::move(temp));
            throw;
          }
        }
        this->has_ = false;
        other.has_ = true;
      }
    } else {
      if (other.has_value()) {
        other.swap(*this);
      } else {
        using std::swap;
        swap(error(), other.error());
      }
    }
  }

  template <typename T2, typename E2>
    requires(!std::is_void_v<T2>)
  friend constexpr bool operator==(const expected& lhs,
                                   const expected<T2, E2>& rhs)
  {
    if (lhs.has_value() != rhs.has_value()) {
      return false;
    }
    if (lhs.has_value()) {
      return lhs.value() == rhs.value();
    }

    return lhs.error() == rhs.error();
  }

  template <typename T2>
  friend constexpr bool operator==(const expected& x, const T2& val)
  {
    return x.has_value() && static_cast<bool>(*x == val);
  }

  template <typename E2>
  friend constexpr bool operator==(const expected& x, const unexpected<E2>& e)
  {
    return !x.has_value() && static_cast<bool>(x.error() == e.error());
  }

  friend constexpr void swap(expected& lhs,
                             expected& rhs) noexcept(noexcept(lhs.swap(rhs)))
  {
    lhs.swap(rhs);
  }

private:
  union {
    T val_;
    E err_;
  };
  bool has_;
};

template <typename E>
class expected<void, E> {
  template <typename U, typename G>
  static constexpr bool is_expected_like_constructible_v
      = std::is_constructible_v<unexpected<E>, expected<U, G>&>
      || std::is_constructible_v<unexpected<E>, expected<U, G>>
      || std::is_constructible_v<unexpected<E>, const expected<U, G>&>
      || std::is_constructible_v<unexpected<E>, const expected<U, G>>;

public:
  static_assert(std::is_same_v<std::remove_cvref_t<E>, E>);
  static_assert(!is_specialization_of_v<std::remove_cvref_t<E>, unexpected>);
  static_assert(std::is_object_v<E>);
  static_assert(std::is_destructible_v<std::remove_cvref_t<E>>);
  static_assert(!std::is_array_v<std::remove_cvref_t<E>>);

  using value_type = void;
  using error_type = E;
  using unexpected_type = unexpected<E>;

  template <typename U>
  using rebind = expected<U, E>;

  constexpr expected()
      : nul_()
      , has_(true)
  {
  }

  constexpr expected(const expected& other)
    requires(!std::is_trivially_copy_constructible_v<E>
             && std::is_copy_constructible_v<E>)
  {
    if (!other) {
      std::construct_at(std::addressof(this->err_), other.error());
    }
    this->has_ = other.has_value();
  }

  constexpr expected(const expected&)
    requires(std::is_trivially_copy_constructible_v<E>)
  = default;

  constexpr expected(expected&& other) noexcept(
      std::is_nothrow_move_constructible_v<E>)
    requires(!std::is_trivially_move_constructible_v<E>
             && std::is_move_constructible_v<E>)
  {
    if (!other) {
      std::construct_at(std::addressof(this->err_), std::move(other.error()));
    }
    this->has_ = other.has_value();
  }

  constexpr expected(expected&&)
    requires(std::is_trivially_move_constructible_v<E>)
  = default;

  template <typename U, typename G>
  constexpr explicit(std::is_convertible_v<const G&, E>)
      expected(const expected<U, G>& other)
    requires(std::is_constructible_v<E, const G&>
             && !is_expected_like_constructible_v<U, G>)
  {
    if (!other) {
      std::construct_at(std::addressof(this->err_), other.error());
    }
    this->has_ = other.has_value();
  }

  template <typename U, typename G>
  constexpr explicit(std::is_convertible_v<G, E>)
      expected(expected<U, G>&& other)
    requires(std::is_constructible_v<E, G>
             && !is_expected_like_constructible_v<U, G>)
  {
    if (!other) {
      std::construct_at(std::addressof(this->err_), std::move(other.error()));
    }
    this->has_ = other.has_value();
  }

  template <typename G>
  constexpr explicit(!std::is_convertible_v<const G&, E>)
      expected(const unexpected<G>& other)
  {
    std::construct_at(std::addressof(this->err_), other.error());
    this->has_ = false;
  }

  template <typename G>
  constexpr explicit(!std::is_convertible_v<G, E>)
      expected(unexpected<G>&& other)
  {
    std::construct_at(std::addressof(this->err_), std::move(other.error()));
    this->has_ = false;
  }

  constexpr explicit expected(std::in_place_t) noexcept
  {
    this->has_ = true;
  }

  template <typename... Args>
  constexpr explicit expected(unexpect_t, Args&&... args)
    requires(std::is_constructible_v<E, Args...>)
  {
    std::construct_at(std::addressof(this->err_), std::forward<Args>(args)...);
    this->has_ = false;
  }

  template <typename U, typename... Args>
  constexpr explicit expected(unexpect_t,
                              std::initializer_list<U> ilist,
                              Args&&... args)
    requires(std::is_constructible_v<E, std::initializer_list<U>&, Args...>)
  {
    std::construct_at(
        std::addressof(this->err_), ilist, std::forward<Args>(args)...);
    this->has_ = false;
  }

  constexpr ~expected()
    requires(!std::is_trivially_destructible_v<E>)
  {
    if (!has_value()) {
      std::destroy_at(std::addressof(this->err_));
    }
  }

  constexpr ~expected()
    requires(std::is_trivially_destructible_v<E>)
  = default;

  constexpr expected& operator=(const expected& other)
    requires(!std::is_trivially_copy_assignable_v<E>
             && std::is_copy_constructible_v<E> && std::is_copy_assignable_v<E>)
  {
    if (has_value()) {
      if (!other) {
        std::construct_at(std::addressof(this->err_), other.error());
        this->has_ = false;
      }
    } else {
      if (other) {
        std::destroy_at(std::addressof(this->err_));
        this->has_ = true;
      } else {
        this->err_ = other.error();
      }
    }

    return *this;
  }

  constexpr expected& operator=(const expected&)
    requires(std::is_trivially_copy_assignable_v<E>)
  = default;

  constexpr expected&
  operator=(expected&& other) noexcept(std::is_nothrow_move_constructible_v<E>
                                       && std::is_nothrow_move_assignable_v<E>)
    requires(!std::is_trivially_move_assignable_v<E>
             && std::is_move_constructible_v<E> && std::is_move_assignable_v<E>)
  {
    if (has_value()) {
      if (!other) {
        std::construct_at(std::addressof(this->err_), std::move(other.error()));
        this->has_ = false;
      }
    } else {
      if (other) {
        std::destroy_at(std::addressof(this->err_));
        this->has_ = true;
      } else {
        this->err_ = std::move(other.error());
      }
    }

    return *this;
  }

  constexpr expected& operator=(expected&&)
    requires(std::is_trivially_move_assignable_v<E>)
  = default;

  template <typename G>
  constexpr expected& operator=(const unexpected<G>& other)
    requires(std::is_constructible_v<E, const G&>
             && std::is_assignable_v<E&, const G&>)
  {
    if (has_value()) {
      std::construct_at(std::addressof(this->err_), other.error());
      this->has_ = false;
    } else {
      this->err_ = other.error();
    }

    return *this;
  }

  template <typename G>
  constexpr expected& operator=(unexpected<G>&& other)
    requires(std::is_constructible_v<E, G> && std::is_assignable_v<E&, G>)
  {
    if (has_value()) {
      std::construct_at(std::addressof(this->err_), std::move(other.error()));
      this->has_ = false;
    } else {
      this->err_ = std::move(other.error());
    }

    return *this;
  }

  constexpr void operator->() const noexcept
  {
    FITORIA_ASSERT(has_value());
  }

  constexpr void operator*() const noexcept
  {
    FITORIA_ASSERT(has_value());
  }

  constexpr explicit operator bool() const noexcept
  {
    return has_value();
  }

  constexpr bool has_value() const noexcept
  {
    return this->has_;
  }

  constexpr void value() const&
  {
    if (!has_value()) {
      FITORIA_THROW_OR(bad_expected_access(error()), std::terminate());
    }
  }

  constexpr void value() &&
  {
    if (!has_value()) {
      FITORIA_THROW_OR(bad_expected_access(std::move(error())),
                       std::terminate());
    }
  }

  constexpr E& error() & noexcept
  {
    FITORIA_ASSERT(!has_value());
    return this->err_;
  }

  constexpr const E& error() const& noexcept
  {
    FITORIA_ASSERT(!has_value());
    return this->err_;
  }

  constexpr E&& error() && noexcept
  {
    FITORIA_ASSERT(!has_value());
    return std::move(this->err_);
  }

  constexpr const E&& error() const&& noexcept
  {
    FITORIA_ASSERT(!has_value());
    return std::move(this->err_);
  }

  template <typename G>
  constexpr E error_or(G&& default_error) const&
    requires(std::is_copy_constructible_v<E> && std::is_convertible_v<G, E>)
  {
    if (has_value()) {
      return static_cast<E>(std::forward<G>(default_error));
    } else {
      return error();
    }
  }

  template <typename G>
  constexpr E error_or(G&& default_error) &&
    requires(std::is_move_constructible_v<E> && std::is_convertible_v<G, E>)
  {
    if (has_value()) {
      return static_cast<E>(std::forward<G>(default_error));
    } else {
      return std::move(error());
    }
  }

  template <typename F> 
  constexpr auto and_then(F&& f) & 
    requires(std::is_copy_constructible_v<E>)
  {
    using U = std::remove_cvref_t<std::invoke_result_t<F>>;
    static_assert(is_specialization_of_v<U, fitoria::expected>);
    static_assert(std::is_same_v<typename U::error_type, E>);
    if (has_value()) {
      return U(std::invoke(std::forward<F>(f)));
    } else {
      return U(unexpect, error());
    }
  }

  template <typename F>
  constexpr auto and_then(F&& f) const&
    requires(std::is_copy_constructible_v<E>)
  {
    using U = std::remove_cvref_t<std::invoke_result_t<F>>;
    static_assert(is_specialization_of_v<U, fitoria::expected>);
    static_assert(std::is_same_v<typename U::error_type, E>);
    if (has_value()) {
      return U(std::invoke(std::forward<F>(f)));
    } else {
      return U(unexpect, error());
    }
  }

  template <typename F>
  constexpr auto and_then(F&& f) &&
    requires(std::is_move_constructible_v<E>)
  {
    using U = std::remove_cvref_t<std::invoke_result_t<F>>;
    static_assert(is_specialization_of_v<U, fitoria::expected>);
    static_assert(std::is_same_v<typename U::error_type, E>);
    if (has_value()) {
      return U(std::invoke(std::forward<F>(f)));
    } else {
      return U(unexpect, std::move(error()));
    }
  }

  template <typename F>
  constexpr auto and_then(F&& f) const&&
    requires(std::is_move_constructible_v<E>)
  {
    using U = std::remove_cvref_t<std::invoke_result_t<F>>;
    static_assert(is_specialization_of_v<U, fitoria::expected>);
    static_assert(std::is_same_v<typename U::error_type, E>);
    if (has_value()) {
      return U(std::invoke(std::forward<F>(f)));
    } else {
      return U(unexpect, std::move(error()));
    }
  }

  template <typename F>
  constexpr auto or_else(F&& f) &
  {
    using G = std::remove_cvref_t<std::invoke_result_t<F, E&>>;
    static_assert(is_specialization_of_v<G, fitoria::expected>);
    static_assert(std::is_same_v<typename G::value_type, void>);
    if (has_value()) {
      return G(std::in_place);
    } else {
      return G(std::invoke(std::forward<F>(f), error()));
    }
  }

  template <typename F>
  constexpr auto or_else(F&& f) const&
  {
    using G = std::remove_cvref_t<std::invoke_result_t<F, const E&>>;
    static_assert(is_specialization_of_v<G, fitoria::expected>);
    static_assert(std::is_same_v<typename G::value_type, void>);
    if (has_value()) {
      return G(std::in_place);
    } else {
      return G(std::invoke(std::forward<F>(f), error()));
    }
  }

  template <typename F>
  constexpr auto or_else(F&& f) &&
  {
    using G = std::remove_cvref_t<std::invoke_result_t<F, E>>;
    static_assert(is_specialization_of_v<G, fitoria::expected>);
    static_assert(std::is_same_v<typename G::value_type, void>);
    if (has_value()) {
      return G(std::in_place);
    } else {
      return G(std::invoke(std::forward<F>(f), std::move(error())));
    }
  }

  template <typename F>
  constexpr auto or_else(F&& f) const&&
  {
    using G = std::remove_cvref_t<std::invoke_result_t<F, const E>>;
    static_assert(is_specialization_of_v<G, fitoria::expected>);
    static_assert(std::is_same_v<typename G::value_type, void>);
    if (has_value()) {
      return G(std::in_place);
    } else {
      return G(std::invoke(std::forward<F>(f), std::move(error())));
    }
  }

  template <typename F>
  constexpr auto transform(F&& f) &
    requires(std::is_copy_constructible_v<E>)
  {
    using U = std::remove_cv_t<std::invoke_result_t<F>>;
    if (has_value()) {
      return expected<U, E>(std::invoke(std::forward<F>(f)));
    } else {
      return expected<U, E>(unexpect, error());
    }
  }

  template <typename F>
  constexpr auto transform(F&& f) const&
    requires(std::is_copy_constructible_v<E>)
  {
    using U = std::remove_cv_t<std::invoke_result_t<F>>;
    if (has_value()) {
      return expected<U, E>(std::invoke(std::forward<F>(f)));
    } else {
      return expected<U, E>(unexpect, error());
    }
  }

  template <typename F>
  constexpr auto transform(F&& f) &&
    requires(std::is_move_constructible_v<E>)
  {
    using U = std::remove_cv_t<std::invoke_result_t<F>>;
    if (has_value()) {
      return expected<U, E>(std::invoke(std::forward<F>(f)));
    } else {
      return expected<U, E>(unexpect, std::move(error()));
    }
  }

  template <typename F>
  constexpr auto transform(F&& f) const&&
    requires(std::is_move_constructible_v<E>)
  {
    using U = std::remove_cv_t<std::invoke_result_t<F>>;
    if (has_value()) {
      return expected<U, E>(std::invoke(std::forward<F>(f)));
    } else {
      return expected<U, E>(unexpect, std::move(error()));
    }
  }

  template <typename F>
  constexpr auto transform_error(F&& f) &
  {
    using G = std::remove_cv_t<std::invoke_result_t<F, E&>>;
    if (has_value()) {
      return expected<void, G>(std::in_place);
    } else {
      return expected<void, G>(unexpect,
                               std::invoke(std::forward<F>(f), error()));
    }
  }

  template <typename F>
  constexpr auto transform_error(F&& f) const&
  {
    using G = std::remove_cv_t<std::invoke_result_t<F, const E&>>;
    if (has_value()) {
      return expected<void, G>(std::in_place);
    } else {
      return expected<void, G>(unexpect,
                               std::invoke(std::forward<F>(f), error()));
    }
  }

  template <typename F>
  constexpr auto transform_error(F&& f) &&
  {
    using G = std::remove_cv_t<std::invoke_result_t<F, E>>;
    if (has_value()) {
      return expected<void, G>(std::in_place);
    } else {
      return expected<void, G>(
          unexpect, std::invoke(std::forward<F>(f), std::move(error())));
    }
  }

  template <typename F>
  constexpr auto transform_error(F&& f) const&&
  {
    using G = std::remove_cv_t<std::invoke_result_t<F, const E>>;
    if (has_value()) {
      return expected<void, G>(std::in_place);
    } else {
      return expected<void, G>(
          unexpect, std::invoke(std::forward<F>(f), std::move(error())));
    }
  }

  constexpr void emplace() noexcept
  {
    if (!has_value()) {
      std::destroy_at(std::addressof(this->err_));
      this->has_ = true;
    }
  }

  void swap(expected& other) noexcept(std::is_nothrow_move_constructible_v<E>
                                      && std::is_nothrow_swappable_v<E>)
    requires(std::is_swappable_v<E> && std::is_move_constructible_v<E>)
  {
    if (has_value()) {
      if (!other.has_value()) {
        std::construct_at(std::addressof(this->err_), std::move(other.error()));
        std::destroy_at(std::addressof(other.error()));
        this->has_ = false;
        other.has_ = true;
      }
    } else {
      if (other.has_value()) {
        other.swap(*this);
      } else {
        using std::swap;
        swap(error(), other.error());
      }
    }
  }

  template <typename T2, typename E2>
    requires(std::is_void_v<T2>)
  friend constexpr bool operator==(const expected& lhs,
                                   const expected<T2, E2>& rhs)
  {
    if (lhs.has_value() != rhs.has_value()) {
      return false;
    }
    if (lhs.has_value()) {
      return true;
    }

    return lhs.error() == rhs.error();
  }

  template <typename E2>
  friend constexpr bool operator==(const expected& x, const unexpected<E2>& e)
  {
    return !x.has_value() && static_cast<bool>(x.error() == e.error());
  }

  friend constexpr void swap(expected& lhs,
                             expected& rhs) noexcept(noexcept(lhs.swap(rhs)))
  {
    lhs.swap(rhs);
  }

private:
  union {
    char nul_;
    E err_;
  };
  bool has_;
};

template <typename T, typename E>
class expected<T&, E> {
  template <typename U, typename G>
  static constexpr bool is_expected_like_constructible_v
      = std::is_constructible_v<T&, expected<U, G>&>
      || std::is_constructible_v<T&, expected<U, G>>
      || std::is_constructible_v<T&, const expected<U, G>&>
      || std::is_constructible_v<T&, const expected<U, G>>
      || std::is_convertible_v<expected<U, G>&, T&>
      || std::is_convertible_v<expected<U, G>, T&>
      || std::is_convertible_v<const expected<U, G>&, T&>
      || std::is_convertible_v<const expected<U, G>, T&>
      || std::is_constructible_v<unexpected<E>, expected<U, G>&>
      || std::is_constructible_v<unexpected<E>, expected<U, G>>
      || std::is_constructible_v<unexpected<E>, const expected<U, G>&>
      || std::is_constructible_v<unexpected<E>, const expected<U, G>>;

public:
  static_assert(!std::is_same_v<std::remove_cvref_t<T>, std::in_place_t>);
  static_assert(!std::is_same_v<std::remove_cvref_t<T>, unexpect_t>);
  static_assert(!is_specialization_of_v<std::remove_cvref_t<T>, unexpected>);
  static_assert(!std::is_array_v<std::remove_cvref_t<T>>);

  static_assert(std::is_same_v<std::remove_cvref_t<E>, E>);
  static_assert(!is_specialization_of_v<std::remove_cvref_t<E>, unexpected>);
  static_assert(std::is_object_v<E>);
  static_assert(std::is_destructible_v<std::remove_cvref_t<E>>);
  static_assert(!std::is_array_v<std::remove_cvref_t<E>>);

  using value_type = T&;
  using error_type = E;
  using unexpected_type = unexpected<E>;

  template <typename U>
  using rebind = expected<U, E>;

  constexpr expected(const expected& other)
    requires(!std::is_trivially_copy_constructible_v<E>
             && std::is_copy_constructible_v<E>)
  {
    if (other) {
      this->valptr_ = other.valptr_;
    } else {
      std::construct_at(std::addressof(this->err_), other.error());
    }
    this->has_ = other.has_value();
  }

  constexpr expected(const expected&)
    requires(std::is_trivially_copy_constructible_v<E>)
  = default;

  constexpr expected(expected&& other) noexcept(
      std::is_nothrow_move_constructible_v<E>)
    requires(!std::is_trivially_move_constructible_v<E>
             && std::is_move_constructible_v<E>)
  {
    if (other) {
      this->valptr_ = other.valptr_;
    } else {
      std::construct_at(std::addressof(this->err_), std::move(other.error()));
    }
    this->has_ = other.has_value();
  }

  constexpr expected(expected&&)
    requires(std::is_trivially_move_constructible_v<E>)
  = default;

  template <typename U, typename G>
  constexpr explicit expected(expected<U, G>& other)
    requires(std::is_constructible_v<T&, const U&>
             && std::is_constructible_v<E, const G&>
             && !is_expected_like_constructible_v<U, G>)
  {
    if (other) {
      this->valptr_ = std::addressof(other.value());
    } else {
      std::construct_at(std::addressof(this->err_), other.error());
    }
    this->has_ = other.has_value();
  }

  template <typename U, typename G>
  constexpr explicit expected(const expected<U, G>& other)
    requires(std::is_constructible_v<T&, const U&>
             && std::is_constructible_v<E, const G&>
             && !is_expected_like_constructible_v<U, G>)
  {
    if (other) {
      this->valptr_ = std::addressof(other.value());
    } else {
      std::construct_at(std::addressof(this->err_), other.error());
    }
    this->has_ = other.has_value();
  }

  template <typename U = T>
  constexpr explicit(!std::is_convertible_v<U, T>) expected(U&& value)
    requires(
        !std::is_same_v<std::remove_cvref_t<U>, std::in_place_t>
        && !is_specialization_of_v<std::remove_cvref_t<U>, fitoria::expected>
        && !is_specialization_of_v<std::remove_cvref_t<U>, unexpected>)
  {
    static_assert(std::is_constructible_v<std::add_lvalue_reference_t<T>, U>);
    static_assert(std::is_lvalue_reference_v<U>);

    this->valptr_ = std::addressof(value);
    this->has_ = true;
  }

  template <typename G>
  constexpr explicit(!std::is_convertible_v<const G&, E>)
      expected(const unexpected<G>& other)
  {
    std::construct_at(std::addressof(this->err_), other.error());
    this->has_ = false;
  }

  template <typename G>
  constexpr explicit(!std::is_convertible_v<G, E>)
      expected(unexpected<G>&& other)
  {
    std::construct_at(std::addressof(this->err_), std::move(other.error()));
    this->has_ = false;
  }

  template <typename U>
  constexpr explicit expected(std::in_place_t, U& value)
  {
    this->valptr_ = std::addressof(value);
    this->has_ = true;
  }

  template <typename... Args>
  constexpr explicit expected(unexpect_t, Args&&... args)
    requires(std::is_constructible_v<E, Args...>)
  {
    std::construct_at(std::addressof(this->err_), std::forward<Args>(args)...);
    this->has_ = false;
  }

  template <typename U, typename... Args>
  constexpr explicit expected(unexpect_t,
                              std::initializer_list<U> ilist,
                              Args&&... args)
    requires(std::is_constructible_v<E, std::initializer_list<U>&, Args...>)
  {
    std::construct_at(
        std::addressof(this->err_), ilist, std::forward<Args>(args)...);
    this->has_ = false;
  }

  constexpr ~expected()
    requires(!std::is_trivially_destructible_v<E>)
  {
    if (!has_value()) {
      std::destroy_at(std::addressof(this->err_));
    }
  }

  constexpr ~expected()
    requires(std::is_trivially_destructible_v<E>)
  = default;

  constexpr expected& operator=(const expected& other)
    requires(!std::is_trivially_copy_assignable_v<E>
             && std::is_copy_constructible_v<E> && std::is_copy_assignable_v<E>)
  {
    if (has_value()) {
      if (other) {
        this->valptr_ = other.valptr_;
      } else {
        std::construct_at(std::addressof(this->err_), other.error());
        this->has_ = false;
      }
    } else {
      if (other) {
        std::destroy_at(std::addressof(this->err_));
        this->valptr_ = other.valptr_;
        this->has_ = true;
      } else {
        this->err_ = other.error();
      }
    }

    return *this;
  }

  constexpr expected& operator=(const expected&)
    requires(std::is_trivially_copy_assignable_v<E>)
  = default;

  constexpr expected&
  operator=(expected&& other) noexcept(std::is_nothrow_move_constructible_v<E>
                                       && std::is_nothrow_move_assignable_v<E>)
    requires(!std::is_trivially_move_assignable_v<E>
             && std::is_move_constructible_v<E> && std::is_move_assignable_v<E>)
  {
    if (has_value()) {
      if (other) {
        this->valptr_ = other.valptr_;
      } else {
        std::construct_at(std::addressof(this->err_), std::move(other.error()));
        this->has_ = false;
      }
    } else {
      if (other) {
        std::destroy_at(std::addressof(this->err_));
        this->valptr_ = other.valptr_;
        this->has_ = true;
      } else {
        this->err_ = std::move(other.error());
      }
    }

    return *this;
  }

  constexpr expected& operator=(expected&&)
    requires(std::is_trivially_move_assignable_v<E>)
  = default;

  template <typename U = T>
  constexpr expected& operator=(U&& value)
    requires(
        !std::is_same_v<std::remove_cvref_t<U>, expected>
        && !is_specialization_of_v<std::remove_cvref_t<U>, fitoria::expected>)
  {
    static_assert(std::is_lvalue_reference_v<U>);

    if (!has_value()) {
      std::destroy_at(std::addressof(this->err_));
    }
    this->valptr_ = std::addressof(value);
    this->has_ = true;

    return *this;
  }

  template <typename G>
  constexpr expected& operator=(const unexpected<G>& other)
    requires(std::is_constructible_v<E, const G&>
             && std::is_assignable_v<E&, const G&>)
  {
    if (has_value()) {
      this->has_ = false;
    } else {
      this->err_ = other.error();
    }

    return *this;
  }

  template <typename G>
  constexpr expected& operator=(unexpected<G>&& other)
    requires(std::is_constructible_v<E, G> && std::is_assignable_v<E&, G>)
  {
    if (has_value()) {
      this->has_ = false;
    } else {
      this->err_ = std::move(other.error());
    }

    return *this;
  }

  constexpr std::add_pointer_t<T> operator->() const noexcept
  {
    FITORIA_ASSERT(has_value());
    return this->valptr_;
  }

  constexpr T& operator*() const noexcept
  {
    FITORIA_ASSERT(has_value());
    return *this->valptr_;
  }

  constexpr explicit operator bool() const noexcept
  {
    return has_value();
  }

  constexpr bool has_value() const noexcept
  {
    return this->has_;
  }

  constexpr T& value() const
  {
    if (!has_value()) {
      FITORIA_THROW_OR(bad_expected_access(error()), std::terminate());
    }

    return *this->valptr_;
  }

  constexpr E& error() & noexcept
  {
    FITORIA_ASSERT(!has_value());
    return this->err_;
  }

  constexpr const E& error() const& noexcept
  {
    FITORIA_ASSERT(!has_value());
    return this->err_;
  }

  constexpr E&& error() && noexcept
  {
    FITORIA_ASSERT(!has_value());
    return std::move(this->err_);
  }

  constexpr const E&& error() const&& noexcept
  {
    FITORIA_ASSERT(!has_value());
    return std::move(this->err_);
  }

  template <typename U>
  constexpr T value_or(U&& default_value) const
    requires(std::is_copy_constructible_v<T> && std::is_convertible_v<U, T>)
  {
    if (has_value()) {
      return value();
    } else {
      return static_cast<T>(std::forward<U>(default_value));
    }
  }

  template <typename G>
  constexpr E error_or(G&& default_error) const&
    requires(std::is_copy_constructible_v<E> && std::is_convertible_v<G, E>)
  {
    if (has_value()) {
      return static_cast<E>(std::forward<G>(default_error));
    } else {
      return error();
    }
  }

  template <typename G>
  constexpr E error_or(G&& default_error) &&
    requires(std::is_move_constructible_v<E> && std::is_convertible_v<G, E>)
  {
    if (has_value()) {
      return static_cast<E>(std::forward<G>(default_error));
    } else {
      return std::move(error());
    }
  }

  template <typename F>
  constexpr auto and_then(F&& f) & 
    requires(std::is_copy_constructible_v<E>)
  {
    using U = std::remove_cvref_t<std::invoke_result_t<F, T&>>;
    static_assert(is_specialization_of_v<U, fitoria::expected>);
    static_assert(std::is_same_v<typename U::error_type, E>);
    if (has_value()) {
      return U(std::invoke(std::forward<F>(f), value()));
    } else {
      return U(unexpect, error());
    }
  }

  template <typename F>
  constexpr auto and_then(F&& f) const&
    requires(std::is_copy_constructible_v<E>)
  {
    using U = std::remove_cvref_t<std::invoke_result_t<F, T&>>;
    static_assert(is_specialization_of_v<U, fitoria::expected>);
    static_assert(std::is_same_v<typename U::error_type, E>);
    if (has_value()) {
      return U(std::invoke(std::forward<F>(f), value()));
    } else {
      return U(unexpect, error());
    }
  }

  template <typename F>
  constexpr auto and_then(F&& f) &&
    requires(std::is_move_constructible_v<E>)
  {
    using U = std::remove_cvref_t<std::invoke_result_t<F, T&>>;
    static_assert(is_specialization_of_v<U, fitoria::expected>);
    static_assert(std::is_same_v<typename U::error_type, E>);
    if (has_value()) {
      return U(std::invoke(std::forward<F>(f), value()));
    } else {
      return U(unexpect, std::move(error()));
    }
  }

  template <typename F>
  constexpr auto and_then(F&& f) const&&
    requires(std::is_move_constructible_v<E>)
  {
    using U = std::remove_cvref_t<std::invoke_result_t<F, T&>>;
    static_assert(is_specialization_of_v<U, fitoria::expected>);
    static_assert(std::is_same_v<typename U::error_type, E>);
    if (has_value()) {
      return U(std::invoke(std::forward<F>(f), value()));
    } else {
      return U(unexpect, std::move(error()));
    }
  }

  template <typename F>
  constexpr auto or_else(F&& f) &
  {
    using G = std::remove_cvref_t<std::invoke_result_t<F, E&>>;
    static_assert(is_specialization_of_v<G, fitoria::expected>);
    static_assert(std::is_same_v<typename G::value_type, T&>);
    if (has_value()) {
      return G(std::in_place, value());
    } else {
      return G(std::invoke(std::forward<F>(f), error()));
    }
  }

  template <typename F>
  constexpr auto or_else(F&& f) const&
  {
    using G = std::remove_cvref_t<std::invoke_result_t<F, const E&>>;
    static_assert(is_specialization_of_v<G, fitoria::expected>);
    static_assert(std::is_same_v<typename G::value_type, T&>);
    if (has_value()) {
      return G(std::in_place, value());
    } else {
      return G(std::invoke(std::forward<F>(f), error()));
    }
  }

  template <typename F>
  constexpr auto or_else(F&& f) &&
  {
    using G = std::remove_cvref_t<std::invoke_result_t<F, E>>;
    static_assert(is_specialization_of_v<G, fitoria::expected>);
    static_assert(std::is_same_v<typename G::value_type, T&>);
    if (has_value()) {
      return G(std::in_place, value());
    } else {
      return G(std::invoke(std::forward<F>(f), std::move(error())));
    }
  }

  template <typename F>
  constexpr auto or_else(F&& f) const&&
  {
    using G = std::remove_cvref_t<std::invoke_result_t<F, const E>>;
    static_assert(is_specialization_of_v<G, fitoria::expected>);
    static_assert(std::is_same_v<typename G::value_type, T&>);
    if (has_value()) {
      return G(std::in_place, value());
    } else {
      return G(std::invoke(std::forward<F>(f), std::move(error())));
    }
  }

  template <typename F>
  constexpr auto transform(F&& f) &
    requires(std::is_copy_constructible_v<E>)
  {
    using U = std::remove_cv_t<std::invoke_result_t<F, T&>>;
    if (has_value()) {
      return expected<U, E>(std::invoke(std::forward<F>(f), value()));
    } else {
      return expected<U, E>(unexpect, error());
    }
  }

  template <typename F>
  constexpr auto transform(F&& f) const&
    requires(std::is_copy_constructible_v<E>)
  {
    using U = std::remove_cv_t<std::invoke_result_t<F, T&>>;
    if (has_value()) {
      return expected<U, E>(std::invoke(std::forward<F>(f), value()));
    } else {
      return expected<U, E>(unexpect, error());
    }
  }

  template <typename F>
  constexpr auto transform(F&& f) &&
    requires(std::is_move_constructible_v<E>)
  {
    using U = std::remove_cv_t<std::invoke_result_t<F, T&>>;
    if (has_value()) {
      return expected<U, E>(std::invoke(std::forward<F>(f), value()));
    } else {
      return expected<U, E>(unexpect, std::move(error()));
    }
  }

  template <typename F>
  constexpr auto transform(F&& f) const&&
    requires(std::is_move_constructible_v<E>)
  {
    using U = std::remove_cv_t<std::invoke_result_t<F, T&>>;
    if (has_value()) {
      return expected<U, E>(std::invoke(std::forward<F>(f), value()));
    } else {
      return expected<U, E>(unexpect, std::move(error()));
    }
  }

  template <typename F>
  constexpr auto transform_error(F&& f) &
  {
    using G = std::remove_cv_t<std::invoke_result_t<F, E&>>;
    if (has_value()) {
      return expected<T, G>(std::in_place, value());
    } else {
      return expected<T, G>(unexpect, std::invoke(std::forward<F>(f), error()));
    }
  }

  template <typename F>
  constexpr auto transform_error(F&& f) const&
  {
    using G = std::remove_cv_t<std::invoke_result_t<F, const E&>>;
    if (has_value()) {
      return expected<T, G>(std::in_place, value());
    } else {
      return expected<T, G>(unexpect, std::invoke(std::forward<F>(f), error()));
    }
  }

  template <typename F>
  constexpr auto transform_error(F&& f) &&
  {
    using G = std::remove_cv_t<std::invoke_result_t<F, E>>;
    if (has_value()) {
      return expected<T, G>(std::in_place, value());
    } else {
      return expected<T, G>(
          unexpect, std::invoke(std::forward<F>(f), std::move(error())));
    }
  }

  template <typename F>
  constexpr auto transform_error(F&& f) const&&
  {
    using G = std::remove_cv_t<std::invoke_result_t<F, const E>>;
    if (has_value()) {
      return expected<T, G>(std::in_place, value());
    } else {
      return expected<T, G>(
          unexpect, std::invoke(std::forward<F>(f), std::move(error())));
    }
  }

  template <typename U>
  constexpr T& emplace(U& value) noexcept
  {
    if (!has_value()) {
      std::destroy_at(std::addressof(this->err_));
    }

    this->valptr_ = std::addressof(value);
    this->has_ = true;
    return *this->valptr_;
  }

  constexpr void swap(expected& other) noexcept(
      std::is_nothrow_move_constructible_v<E> && std::is_nothrow_swappable_v<E>)
  {
    if (has_value()) {
      if (other.has_value()) {
        using std::swap;
        swap(this->valptr_, other.valptr_);
      } else {
        auto* valptr = this->valptr_;
        std::construct_at(std::addressof(this->err_), std::move(other.err_));
        std::destroy_at(std::addressof(other.err_));
        other.valptr_ = valptr;
        std::swap(this->has_, other.has_);
      }
    } else {
      if (other.has_value()) {
        auto* valptr = other.valptr_;
        std::construct_at(std::addressof(other.err_), std::move(this->err_));
        std::destroy_at(std::addressof(this->err_));
        this->valptr_ = valptr;
        std::swap(this->has_, other.has_);
      } else {
        using std::swap;
        swap(error(), other.error());
      }
    }
  }

  template <typename T2, typename E2>
    requires(!std::is_void_v<T2>)
  friend constexpr bool operator==(const expected& lhs,
                                   const expected<T2, E2>& rhs)
  {
    if (lhs.has_value() != rhs.has_value()) {
      return false;
    }
    if (lhs.has_value()) {
      return lhs.value() == rhs.value();
    }

    return lhs.error() == rhs.error();
  }

  template <typename T2>
  friend constexpr bool operator==(const expected& x, const T2& val)
  {
    return x.has_value() && static_cast<bool>(*x == val);
  }

  template <typename E2>
  friend constexpr bool operator==(const expected& x, const unexpected<E2>& e)
  {
    return !x.has_value() && static_cast<bool>(x.error() == e.error());
  }

  friend void constexpr swap(expected& lhs,
                             expected& rhs) noexcept(noexcept(lhs.swap(rhs)))
    requires std::is_swappable_v<T> && std::is_swappable_v<E>
  {
    lhs.swap(rhs);
  }

private:
  union {
    std::remove_reference_t<T>* valptr_;
    E err_;
  };
  bool has_;
};

FITORIA_NAMESPACE_END

template <typename T, typename E, typename CharT>
struct FITORIA_NAMESPACE::fmt::formatter<FITORIA_NAMESPACE::expected<T, E>,
                                         CharT>
    : FITORIA_NAMESPACE::fmt::formatter<std::remove_cvref_t<T>, CharT> {
  template <typename FormatContext>
  auto format(FITORIA_NAMESPACE::expected<T, E> exp, FormatContext& ctx) const
  {
    if (exp) {
      return FITORIA_NAMESPACE::fmt::formatter<std::remove_cvref_t<T>,
                                               CharT>::format(exp.value(), ctx);
    } else {
      return FITORIA_NAMESPACE::fmt::format_to(ctx.out(), "{{unexpected}}");
    }
  }
};

#endif
