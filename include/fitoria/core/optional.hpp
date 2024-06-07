//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_CORE_OPTIONAL_HPP
#define FITORIA_CORE_OPTIONAL_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/core/format.hpp>
#include <fitoria/core/type_traits.hpp>

#include <functional>
#include <initializer_list>
#include <memory>
#include <utility>

FITORIA_NAMESPACE_BEGIN

struct nullopt_t {
  struct tag_t {
    constexpr explicit tag_t() noexcept = default;
  };

  constexpr explicit nullopt_t(tag_t) noexcept { }
};

inline constexpr nullopt_t nullopt { nullopt_t::tag_t {} };

#if !FITORIA_NO_EXCEPTIONS

class bad_optional_access : public std::exception {
public:
  ~bad_optional_access() noexcept override = default;

  const char* what() const noexcept override
  {
    return "bad optional access";
  }
};

#endif

template <typename T>
class optional {
  template <typename U>
  static constexpr bool is_optional_like_constructible_v
      = std::is_constructible_v<T, optional<U>&>
      || std::is_constructible_v<T, const optional<U>&>
      || std::is_constructible_v<T, optional<U>&&>
      || std::is_constructible_v<T, const optional<U>&&>
      || std::is_convertible_v<optional<U>&, T>
      || std::is_convertible_v<const optional<U>&, T>
      || std::is_convertible_v<optional<U>&&, T>
      || std::is_convertible_v<const optional<U>&&, T>;

  template <typename U>
  static constexpr bool is_optional_like_assignable_v
      = is_optional_like_constructible_v<U>
      || std::is_assignable_v<T&, optional<U>&>
      || std::is_assignable_v<T&, const optional<U>&>
      || std::is_assignable_v<T&, optional<U>&&>
      || std::is_assignable_v<T&, const optional<U>&&>;

public:
  static_assert(!std::is_same_v<std::remove_cvref_t<T>, std::in_place_t>);
  static_assert(!std::is_same_v<std::remove_cvref_t<T>, nullopt_t>);
  static_assert(!std::is_array_v<std::remove_cvref_t<T>>);
  static_assert(!std::is_reference_v<std::remove_cvref_t<T>>);
  static_assert(std::is_destructible_v<std::remove_cvref_t<T>>);

  using value_type = T;

  constexpr optional() noexcept
      : has_(false)
  {
  }

  constexpr optional(nullopt_t) noexcept {};

  constexpr optional(const optional& other)
    requires(!std::is_trivially_copy_constructible_v<T>
             && std::is_copy_constructible_v<T>)
  {
    if (other) {
      std::construct_at(std::addressof(this->val_), *other);
      this->has_ = true;
    }
  }

  constexpr optional(const optional&)
    requires(std::is_trivially_copy_constructible_v<T>)
  = default;

  constexpr optional(optional&& other) noexcept(
      std::is_nothrow_move_constructible_v<T>)
    requires(!std::is_trivially_move_constructible_v<T>
             && std::is_move_constructible_v<T>)
  {
    if (other) {
      std::construct_at(std::addressof(this->val_), std::move(*other));
      this->has_ = true;
    }
  }

  constexpr optional(optional&&)
    requires(std::is_trivially_move_constructible_v<T>)
  = default;

  template <typename U>
  constexpr explicit(!std::is_convertible_v<const U&, T>)
      optional(const optional<U>& other)
    requires(std::is_constructible_v<T, const U&>
             && !is_optional_like_constructible_v<U>)
  {
    if (other) {
      std::construct_at(std::addressof(this->val_), *other);
      this->has_ = true;
    }
  }

  template <typename U>
  constexpr explicit(!std::is_convertible_v<U&&, T>)
      optional(optional<U>&& other)
    requires(std::is_constructible_v<T, U &&>
             && !is_optional_like_constructible_v<U>)
  {
    if (other) {
      std::construct_at(std::addressof(this->val_), std::move(*other));
      this->has_ = true;
    }
  }

  template <typename... Args>
  constexpr explicit optional(std::in_place_t, Args&&... args)
    requires(std::is_constructible_v<T, Args...>)
  {
    std::construct_at(std::addressof(this->val_), std::forward<Args>(args)...);
    this->has_ = true;
  }

  template <typename U, typename... Args>
  constexpr explicit optional(std::in_place_t,
                              std::initializer_list<U> ilist,
                              Args&&... args)
    requires(std::is_constructible_v<T, std::initializer_list<U>&, Args && ...>)
  {
    std::construct_at(
        std::addressof(this->val_), ilist, std::forward<Args>(args)...);
    this->has_ = true;
  }

  template <typename U = T>
  constexpr explicit(!std::is_convertible_v<U&&, T>) optional(U&& value)
    requires(
        std::is_constructible_v<T, U &&>
        && !std::is_same_v<std::remove_cvref_t<U>, std::in_place_t>
        && !is_specialization_of_v<std::remove_cvref_t<U>, fitoria::optional>)
  {
    std::construct_at(std::addressof(this->val_), std::forward<U>(value));
    this->has_ = true;
  }

  constexpr ~optional()
    requires(!std::is_trivially_destructible_v<T>)
  {
    if (has_value()) {
      std::destroy_at(std::addressof(this->val_));
    }
  }

  constexpr ~optional()
    requires(std::is_trivially_destructible_v<T>)
  = default;

  constexpr optional& operator=(nullopt_t) noexcept
  {
    if (has_value()) {
      std::destroy_at(std::addressof(this->val_));
      this->has_ = false;
    }

    return *this;
  }

  constexpr optional& operator=(const optional& other)
    requires(!(std::is_trivially_copy_constructible_v<T>
               && std::is_trivially_copy_assignable_v<T>)
             && std::is_copy_constructible_v<T> && std::is_copy_assignable_v<T>)
  {
    if (has_value()) {
      if (other.has_value()) {
        this->val_ = *other;
      } else {
        std::destroy_at(std::addressof(this->val_));
        this->has_ = false;
      }
    } else {
      if (other.has_value()) {
        std::construct_at(std::addressof(this->val_), *other);
        this->has_ = true;
      }
    }

    return *this;
  }

  constexpr optional& operator=(const optional&)
    requires(std::is_trivially_copy_constructible_v<T>
             && std::is_trivially_copy_assignable_v<T>)
  = default;

  constexpr optional& operator=(optional&& other)
    requires(!(std::is_trivially_move_constructible_v<T>
               && std::is_trivially_move_assignable_v<T>)
             && std::is_move_constructible_v<T> && std::is_move_assignable_v<T>)
  {
    if (has_value()) {
      if (other.has_value()) {
        this->val_ = std::move(*other);
      } else {
        std::destroy_at(std::addressof(this->val_));
        this->has_ = false;
      }
    } else {
      if (other.has_value()) {
        std::construct_at(std::addressof(this->val_), std::move(*other));
        this->has_ = true;
      }
    }

    return *this;
  }

  constexpr optional& operator=(optional&&)
    requires(std::is_trivially_move_constructible_v<T>
             && std::is_trivially_move_assignable_v<T>)
  = default;

  template <typename U = T>
  constexpr optional& operator=(U&& value)
    requires(std::is_constructible_v<T, U> && std::is_assignable_v<T&, U>
             && !std::is_same_v<std::remove_cvref_t<U>, std::in_place_t>
             && !std::is_same_v<std::remove_cvref_t<U>, optional>
             && (!std::is_scalar_v<T> || !std::is_same_v<std::decay_t<U>, T>))
  {
    if (has_value()) {
      this->val_ = std::forward<U>(value);
    } else {
      std::construct_at(std::addressof(this->val_), std::forward<U>(value));
      this->has_ = true;
    }

    return *this;
  }

  template <typename U>
  constexpr optional& operator=(const optional<U>& other)
    requires(std::is_constructible_v<T, const U&>
             && std::is_assignable_v<T&, const U&>
             && !is_optional_like_assignable_v<U>)
  {
    if (has_value()) {
      if (other) {
        this->val_ = *other;
      } else {
        std::destroy_at(std::addressof(this->val_));
        this->has_ = false;
      }
    } else {
      if (other) {
        std::construct_at(std::addressof(this->val_), *other);
        this->has_ = true;
      }
    }

    return *this;
  }

  template <typename U>
  constexpr optional& operator=(optional<U>&& other)
    requires(std::is_constructible_v<T, U> && std::is_assignable_v<T&, U>
             && !is_optional_like_assignable_v<U>)
  {
    if (has_value()) {
      if (other) {
        this->val_ = std::move(*other);
      } else {
        std::destroy_at(std::addressof(this->val_));
        this->has_ = false;
      }
    } else {
      if (other) {
        std::construct_at(std::addressof(this->val_), std::move(*other));
        this->has_ = true;
      }
    }

    return *this;
  }

  template <typename... Args>
  constexpr T& emplace(Args&&... args)
  {
    if (has_value()) {
      std::destroy_at(std::addressof(this->val_));
      this->has_ = false;
    }

    std::construct_at(std::addressof(this->val_), std::forward<Args>(args)...);
    this->has_ = true;
    return this->val_;
  }

  template <typename U, typename... Args>
  constexpr T& emplace(std::initializer_list<U> ilist, Args&&... args)
    requires(std::is_constructible_v<T, std::initializer_list<U>&, Args...>)
  {
    if (has_value()) {
      std::destroy_at(std::addressof(this->val_));
      this->has_ = false;
    }

    std::construct_at(
        std::addressof(this->val_), ilist, std::forward<Args>(args)...);
    this->has_ = true;
    return this->val_;
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
      FITORIA_THROW_OR(bad_optional_access(), std::terminate());
    }

    return this->val_;
  }

  constexpr const T& value() const&
  {
    if (!has_value()) {
      FITORIA_THROW_OR(bad_optional_access(), std::terminate());
    }

    return this->val_;
  }

  constexpr T&& value() &&
  {
    if (!has_value()) {
      FITORIA_THROW_OR(bad_optional_access(), std::terminate());
    }

    return std::move(this->val_);
  }

  constexpr const T&& value() const&&
  {
    if (!has_value()) {
      FITORIA_THROW_OR(bad_optional_access(), std::terminate());
    }

    return std::move(this->val_);
  }

  template <typename U>
  constexpr T value_or(U&& default_value) const&
    requires(std::is_copy_constructible_v<T>)
  {
    if (has_value()) {
      return this->val_;
    } else {
      return static_cast<T>(std::forward<U>(default_value));
    }
  }

  template <typename U>
  constexpr T value_or(U&& default_value) &&
    requires(std::is_move_constructible_v<T>)
  {
    if (has_value()) {
      return std::move(this->val_);
    } else {
      return static_cast<T>(std::forward<U>(default_value));
    }
  }

  template <typename F>
  constexpr auto and_then(F&& f) &
  {
    using U = std::remove_cvref_t<std::invoke_result_t<F, T&>>;
    static_assert(is_specialization_of_v<U, optional>);
    if (has_value()) {
      return U(std::invoke(std::forward<F>(f), **this));
    } else {
      return U();
    }
  }

  template <typename F>
  constexpr auto and_then(F&& f) const&
  {
    using U = std::remove_cvref_t<std::invoke_result_t<F, const T&>>;
    static_assert(is_specialization_of_v<U, optional>);
    if (has_value()) {
      return U(std::invoke(std::forward<F>(f), **this));
    } else {
      return U();
    }
  }

  template <typename F>
  constexpr auto and_then(F&& f) &&
  {
    using U = std::remove_cvref_t<std::invoke_result_t<F, T>>;
    static_assert(is_specialization_of_v<U, optional>);
    if (has_value()) {
      return U(std::invoke(std::forward<F>(f), std::move(**this)));
    } else {
      return U();
    }
  }

  template <typename F>
  constexpr auto and_then(F&& f) const&&
  {
    using U = std::remove_cvref_t<std::invoke_result_t<F, const T>>;
    static_assert(is_specialization_of_v<U, optional>);
    if (has_value()) {
      return U(std::invoke(std::forward<F>(f), std::move(**this)));
    } else {
      return U();
    }
  }

  template <typename F>
  constexpr auto transform(F&& f) &
  {
    using U = std::remove_cv_t<std::invoke_result_t<F, T&>>;
    if (has_value()) {
      return optional<U>(std::invoke(std::forward<F>(f), **this));
    } else {
      return optional<U>();
    }
  }

  template <typename F>
  constexpr auto transform(F&& f) const&
  {
    using U = std::remove_cv_t<std::invoke_result_t<F, const T&>>;
    if (has_value()) {
      return optional<U>(std::invoke(std::forward<F>(f), **this));
    } else {
      return optional<U>();
    }
  }

  template <typename F>
  constexpr auto transform(F&& f) &&
  {
    using U = std::remove_cv_t<std::invoke_result_t<F, T>>;
    if (has_value()) {
      return optional<U>(std::invoke(std::forward<F>(f), std::move(**this)));
    } else {
      return optional<U>();
    }
  }

  template <typename F>
  constexpr auto transform(F&& f) const&&
  {
    using U = std::remove_cv_t<std::invoke_result_t<F, const T>>;
    if (has_value()) {
      return optional<U>(std::invoke(std::forward<F>(f), std::move(**this)));
    } else {
      return optional<U>();
    }
  }

  template <typename F>
  constexpr auto or_else(F&& f) const&
    requires(std::is_copy_constructible_v<T>)
  {
    using U = std::remove_cvref_t<std::invoke_result_t<F>>;
    static_assert(std::is_same_v<U, optional>);
    if (has_value()) {
      return U(**this);
    } else {
      return U(std::forward<F>(f)());
    }
  }

  template <typename F>
  constexpr auto or_else(F&& f) &&
    requires(std::is_move_constructible_v<T>)
  {
    using U = std::remove_cvref_t<std::invoke_result_t<F>>;
    static_assert(std::is_same_v<U, optional>);
    if (has_value()) {
      return U(std::move(**this));
    } else {
      return U(std::forward<F>(f)());
    }
  }

  constexpr void
  swap(optional& other) noexcept(std::is_nothrow_move_constructible_v<T>
                                 && std::is_nothrow_swappable_v<T>)
    requires(std::is_move_constructible_v<T>)
  {
    if (has_value()) {
      if (other.has_value()) {
        using std::swap;
        swap(this->val_, *other);
      } else {
        std::construct_at(std::addressof(other.val_), std::move(**this));
        std::destroy_at(std::addressof(this->val_));
        this->has_ = false;
        other.has_ = true;
      }
    } else {
      if (other.has_value()) {
        other.swap(*this);
      }
    }
  }

  constexpr void reset() noexcept
  {
    if (has_value()) {
      std::destroy_at(std::addressof(this->val_));
      this->has_ = false;
    }
  }

private:
  union {
    T val_;
  };
  bool has_ = false;
};

template <>
class optional<void> {
public:
  using value_type = void;

  constexpr optional() noexcept = default;

  constexpr optional(nullopt_t) noexcept {};

  constexpr optional(const optional&) noexcept = default;

  constexpr optional(optional&&) noexcept = default;

  constexpr explicit optional(std::in_place_t) noexcept
  {
    this->has_ = true;
  }

  constexpr ~optional() noexcept = default;

  constexpr optional& operator=(nullopt_t) noexcept
  {
    this->has_ = false;

    return *this;
  }

  constexpr optional& operator=(const optional&) noexcept = default;

  constexpr optional& operator=(optional&&) noexcept = default;

  constexpr void emplace() noexcept
  {
    this->has_ = true;
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

  constexpr void value() const
  {
    if (!has_value()) {
      FITORIA_THROW_OR(bad_optional_access(), std::terminate());
    }
  }

  template <typename F>
  constexpr auto and_then(F&& f) const
  {
    static_assert(is_specialization_of_v<std::invoke_result_t<F>, optional>);
    if (has_value()) {
      return std::invoke(std::forward<F>(f));
    } else {
      return std::remove_cvref_t<std::invoke_result_t<F>>();
    }
  }

  template <typename F>
  constexpr auto transform(F&& f) const
  {
    using U = std::remove_cv_t<std::invoke_result_t<F>>;
    if (has_value()) {
      std::forward<F>(f)();
      return optional<U>(std::in_place);
    } else {
      return optional<U>();
    }
  }

  template <typename F>
  constexpr auto or_else(F&& f) const
  {
    using U = std::remove_cvref_t<std::invoke_result_t<F>>;
    static_assert(std::is_same_v<U, optional>);
    if (has_value()) {
      return U(std::in_place);
    } else {
      return U(std::forward<F>(f)());
    }
  }

  constexpr void swap(optional& other) noexcept
  {
    using std::swap;
    swap(this->has_, other.has_);
  }

  constexpr void reset() noexcept
  {
    this->has_ = false;
  }

private:
  bool has_ = false;
};

template <typename T>
class optional<T&> {
public:
  static_assert(!std::is_same_v<std::remove_cvref_t<T>, std::in_place_t>);
  static_assert(!std::is_same_v<std::remove_cvref_t<T>, nullopt_t>);
  static_assert(!std::is_array_v<std::remove_cvref_t<T>>);

  using value_type = T&;

  constexpr optional() noexcept = default;

  constexpr optional(nullopt_t) noexcept {};

  constexpr optional(const optional&) noexcept = default;

  constexpr optional(optional&&) noexcept = default;

  template <typename U = T>
  constexpr explicit(!std::is_convertible_v<U, T>) optional(U&& value) noexcept
    requires(!is_specialization_of_v<std::decay_t<U>, fitoria::optional>)
  {
    static_assert(std::is_constructible_v<std::add_lvalue_reference_t<T>, U>);
    static_assert(std::is_lvalue_reference_v<U>);

    this->valptr_ = std::addressof(value);
  }

  template <typename U>
  constexpr explicit(!std::is_convertible_v<U, T>)
      optional(const optional<U>& other) noexcept
    requires std::is_convertible_v<U, T>
  {
    if (other) {
      this->valptr_ = std::addressof(*other);
    }
  }

  constexpr ~optional() noexcept = default;

  constexpr optional& operator=(nullopt_t) noexcept
  {
    this->valptr_ = nullptr;

    return *this;
  }

  constexpr optional& operator=(const optional&) noexcept = default;

  constexpr optional& operator=(optional&&) noexcept = default;

  template <typename U = T>
  constexpr optional& operator=(U&& value) noexcept
    requires(!is_specialization_of_v<std::decay_t<U>, fitoria::optional>)
  {
    static_assert(std::is_constructible_v<std::add_lvalue_reference_t<T>, U>);
    static_assert(std::is_lvalue_reference_v<U>);

    this->valptr_ = std::addressof(value);

    return *this;
  }

  template <typename U>
  constexpr optional& operator=(const optional<U>& other) noexcept
  {
    static_assert(std::is_constructible_v<std::add_lvalue_reference_t<T>, U>);
    static_assert(std::is_lvalue_reference_v<U>);

    if (other) {
      this->valptr_ = std::addressof(*other);
    }

    return *this;
  }

  template <typename U = T>
  constexpr T& emplace(U&& u) noexcept
    requires(!is_specialization_of_v<std::decay_t<U>, fitoria::optional>)
  {
    static_assert(std::is_lvalue_reference_v<U>);

    this->valptr_ = std::addressof(std::forward<U>(u));

    return *this->valptr_;
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
    return this->valptr_ != nullptr;
  }

  constexpr T& value() const
  {
    if (!has_value()) {
      FITORIA_THROW_OR(bad_optional_access(), std::terminate());
    }

    return *this->valptr_;
  }

  template <typename U>
  constexpr T value_or(U&& default_value) const
    requires(std::is_copy_constructible_v<T>)
  {
    if (has_value()) {
      return **this;
    } else {
      return static_cast<T>(std::forward<U>(default_value));
    }
  }

  template <typename F>
  constexpr auto and_then(F&& f) const
  {
    using U = std::invoke_result_t<F, std::add_lvalue_reference_t<T>>;
    static_assert(is_specialization_of_v<U, optional>);
    if (has_value()) {
      return U(std::invoke(std::forward<F>(f), **this));
    } else {
      return U();
    }
  }

  template <typename F>
  constexpr auto transform(F&& f) const
  {
    using U = std::remove_cv_t<
        std::invoke_result_t<F, std::add_lvalue_reference_t<T>>>;
    if (has_value()) {
      return optional<U>(std::invoke(std::forward<F>(f), **this));
    } else {
      return optional<U>();
    }
  }

  template <typename F>
  constexpr auto or_else(F&& f) const
    requires(std::is_copy_constructible_v<T>)
  {
    using U = std::remove_cvref_t<std::invoke_result_t<F>>;
    static_assert(std::is_same_v<U, optional>);
    if (has_value()) {
      return U(**this);
    } else {
      return U(std::forward<F>(f)());
    }
  }

  constexpr void reset() noexcept
  {
    this->valptr_ = nullptr;
  }

  constexpr void swap(optional& other)
  {
    std::swap(this->valptr_, other.valptr_);
  }

private:
  T* valptr_ = nullptr;
};

template <typename T1, typename T2>
constexpr bool operator==(const optional<T1>& lhs, const optional<T2>& rhs)
  requires(!std::is_void_v<T1> && !std::is_void_v<T2>)
{
  if (lhs.has_value() != rhs.has_value()) {
    return false;
  }

  if (!lhs) {
    return true;
  }

  return *lhs == *rhs;
}

template <typename T1, typename T2>
constexpr bool operator!=(const optional<T1>& lhs, const optional<T2>& rhs)
  requires(!std::is_void_v<T1> && !std::is_void_v<T2>)
{
  if (lhs.has_value() != rhs.has_value()) {
    return true;
  }

  if (!lhs) {
    return false;
  }

  return *lhs != *rhs;
}

template <typename T1, typename T2>
constexpr bool operator<(const optional<T1>& lhs, const optional<T2>& rhs)
  requires(!std::is_void_v<T1> && !std::is_void_v<T2>)
{
  if (!rhs) {
    return false;
  }
  if (!lhs) {
    return true;
  }

  return *lhs < *rhs;
}

template <typename T1, typename T2>
constexpr bool operator<=(const optional<T1>& lhs, const optional<T2>& rhs)
  requires(!std::is_void_v<T1> && !std::is_void_v<T2>)
{
  if (!lhs) {
    return true;
  }
  if (!rhs) {
    return false;
  }

  return *lhs <= *rhs;
}

template <typename T1, typename T2>
constexpr bool operator>(const optional<T1>& lhs, const optional<T2>& rhs)
  requires(!std::is_void_v<T1> && !std::is_void_v<T2>)
{
  if (!lhs) {
    return false;
  }
  if (!rhs) {
    return true;
  }

  return *lhs > *rhs;
}

template <typename T1, typename T2>
constexpr bool operator>=(const optional<T1>& lhs, const optional<T2>& rhs)
  requires(!std::is_void_v<T1> && !std::is_void_v<T2>)
{
  if (!rhs) {
    return true;
  }
  if (!lhs) {
    return false;
  }

  return *lhs >= *rhs;
}

template <typename T1, typename T2>
constexpr bool operator==(const optional<T1>& lhs, const optional<T2>& rhs)
  requires(std::is_void_v<T1> && std::is_void_v<T2>)
{
  return lhs.has_value() == rhs.has_value();
}

template <typename T1, typename T2>
constexpr bool operator!=(const optional<T1>& lhs, const optional<T2>& rhs)
  requires(std::is_void_v<T1> && std::is_void_v<T2>)
{
  return lhs.has_value() != rhs.has_value();
}

template <typename T1, typename T2>
constexpr bool operator<(const optional<T1>& lhs, const optional<T2>& rhs)
  requires(std::is_void_v<T1> && std::is_void_v<T2>)
{
  return rhs && !lhs;
}

template <typename T1, typename T2>
constexpr bool operator<=(const optional<T1>& lhs, const optional<T2>& rhs)
  requires(std::is_void_v<T1> && std::is_void_v<T2>)
{
  return !lhs || rhs;
}

template <typename T1, typename T2>
constexpr bool operator>(const optional<T1>& lhs, const optional<T2>& rhs)
  requires(std::is_void_v<T1> && std::is_void_v<T2>)
{
  return lhs && !rhs;
}

template <typename T1, typename T2>
constexpr bool operator>=(const optional<T1>& lhs, const optional<T2>& rhs)
  requires(std::is_void_v<T1> && std::is_void_v<T2>)
{
  return lhs || !rhs;
}

template <typename T, std::three_way_comparable_with<T> U>
constexpr std::compare_three_way_result_t<T, U>
operator<=>(const optional<T>& lhs, const optional<U>& rhs)
{
  return *lhs <=> *rhs;
}

template <typename T>
constexpr bool operator==(const optional<T>& opt, nullopt_t) noexcept
{
  return !opt;
}

template <typename T>
constexpr std::strong_ordering operator<=>(const optional<T>& opt,
                                           nullopt_t) noexcept
{
  return bool(opt) <=> false;
}

template <typename T, typename U>
constexpr bool operator==(const optional<T>& opt, const U& val)
  requires(!std::is_void_v<T> && !std::is_same_v<U, nullopt_t>)
{
  return opt && *opt == val;
}

template <typename T, typename U>
constexpr bool operator==(const T& val, const optional<U>& opt)
  requires(!std::is_same_v<T, nullopt_t> && !std::is_void_v<U>)
{
  return opt && val == *opt;
}

template <typename T, typename U>
constexpr bool operator!=(const optional<T>& opt, const U& val)
  requires(!std::is_void_v<T> && !std::is_same_v<U, nullopt_t>)
{
  return !opt || *opt != val;
}

template <typename T, typename U>
constexpr bool operator!=(const T& val, const optional<U>& opt)
  requires(!std::is_same_v<T, nullopt_t> && !std::is_void_v<U>)
{
  return !opt || val != *opt;
}

template <typename T, typename U>
constexpr bool operator<(const optional<T>& opt, const U& val)
  requires(!std::is_void_v<T> && !std::is_same_v<U, nullopt_t>)
{
  return !opt || *opt < val;
}

template <typename T, typename U>
constexpr bool operator<(const T& val, const optional<U>& opt)
  requires(!std::is_same_v<T, nullopt_t> && !std::is_void_v<U>)
{
  return opt && val < *opt;
}

template <typename T, typename U>
constexpr bool operator<=(const optional<T>& opt, const U& val)
  requires(!std::is_void_v<T> && !std::is_same_v<U, nullopt_t>)
{
  return !opt || *opt <= val;
}

template <typename T, typename U>
constexpr bool operator<=(const T& val, const optional<U>& opt)
  requires(!std::is_same_v<T, nullopt_t> && !std::is_void_v<U>)
{
  return opt && val <= *opt;
}

template <typename T, typename U>
constexpr bool operator>(const optional<T>& opt, const U& val)
  requires(!std::is_void_v<T> && !std::is_same_v<U, nullopt_t>)
{
  return !(opt <= val);
}

template <typename T, typename U>
constexpr bool operator>(const T& val, const optional<U>& opt)
  requires(!std::is_same_v<T, nullopt_t> && !std::is_void_v<U>)
{
  return !(val <= opt);
}

template <typename T, typename U>
constexpr bool operator>=(const optional<T>& opt, const U& val)
  requires(!std::is_void_v<T> && !std::is_same_v<U, nullopt_t>)
{
  return !(opt < val);
}

template <typename T, typename U>
constexpr bool operator>=(const T& val, const optional<U>& opt)
  requires(!std::is_same_v<T, nullopt_t> && !std::is_void_v<U>)
{
  return !(val < opt);
}

template <typename T>
constexpr optional<std::decay_t<T>> make_optional(T&& value)
{
  return optional<std::decay_t<T>>(std::forward<T>(value));
}

template <typename T, typename... Args>
constexpr optional<T> make_optional(Args&&... args)
{
  return optional<T>(std::in_place, std::forward<Args>(args)...);
}

template <typename T, typename U, typename... Args>
constexpr optional<T> make_optional(std::initializer_list<U> ilist,
                                    Args&&... args)
{
  return optional<T>(
      std::in_place, std::move(ilist), std::forward<Args>(args)...);
}

template <typename T>
constexpr void swap(optional<T>& lhs,
                    optional<T>& rhs) noexcept(noexcept(lhs.swap(rhs)))
  requires(std::is_void_v<T>
           || (std::is_move_constructible_v<T> && std::is_swappable_v<T>))
{
  lhs.swap(rhs);
}

FITORIA_NAMESPACE_END

template <typename T, typename CharT>
struct FITORIA_NAMESPACE::fmt::formatter<FITORIA_NAMESPACE::optional<T>, CharT>
    : FITORIA_NAMESPACE::fmt::formatter<std::remove_cvref_t<T>, CharT> {
  template <typename FormatContext>
  auto format(FITORIA_NAMESPACE::optional<T> opt, FormatContext& ctx) const
  {
    if (opt) {
      return FITORIA_NAMESPACE::fmt::formatter<std::remove_cvref_t<T>,
                                               CharT>::format(*opt, ctx);
    } else {
      return FITORIA_NAMESPACE::fmt::format_to(ctx.out(), "{{nullopt}}");
    }
  }
};

#endif
