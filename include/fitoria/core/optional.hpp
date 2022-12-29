//
// Copyright (c) 2022 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include <fitoria/core/config.hpp>

#include <fitoria/core/type_traits.hpp>

#include <fitoria/core/utility.hpp>

#include <initializer_list>
#include <utility>

FITORIA_NAMESPACE_BEGIN

struct nullopt_t {
  struct tag_t {
    constexpr explicit tag_t() noexcept = default;
  };

  constexpr explicit nullopt_t(tag_t) noexcept { }
};

inline constexpr nullopt_t nullopt { nullopt_t::tag_t {} };

class bad_optional_access : public std::exception {
public:
  ~bad_optional_access() noexcept override = default;

  const char* what() const noexcept override
  {
    return "[bad_optional_access]";
  }
};

template <typename T>
class optional_base {
public:
  constexpr optional_base() noexcept
      : nul_()
      , has_(false)
  {
  }

  template <typename U>
  constexpr optional_base(std::in_place_t, U&& value)
      : val_(std::forward<U>(value))
      , has_(true)
  {
  }

  constexpr ~optional_base()
    requires(!std::is_trivially_destructible_v<T>)
  {
    if (has_) {
      val_.~T();
    }
  }

  constexpr ~optional_base()
    requires(std::is_trivially_destructible_v<T>)
  = default;

protected:
  union {
    char nul_;
    T val_;
  };
  bool has_ = false;
};

template <typename T>
  requires(std::is_reference_v<T>)
class optional_base<T> {
public:
  constexpr optional_base() noexcept = default;

  template <typename U>
  constexpr optional_base(std::in_place_t, U& value)
      : valptr_(std::addressof(value))
  {
  }

protected:
  std::remove_reference_t<T>* valptr_ = nullptr;
};

template <typename T>
class optional : public optional_base<T> {
  using base_type = optional_base<T>;

  static constexpr bool is_reference_v = std::is_reference_v<T>;

  template <typename U>
  static constexpr bool is_optional_like_construct_v
      = std::is_constructible_v<T, optional<U>&>
      || std::is_constructible_v<T, const optional<U>&>
      || std::is_constructible_v<T, optional<U>&&>
      || std::is_constructible_v<T, const optional<U>&&>
      || std::is_convertible_v<optional<U>&, T>
      || std::is_convertible_v<const optional<U>&, T>
      || std::is_convertible_v<optional<U>&&, T>
      || std::is_convertible_v<const optional<U>&&, T>;

  template <typename U>
  static constexpr bool is_optional_like_assign_v
      = is_optional_like_construct_v<U>
      || std::is_assignable_v<T&, optional<U>&>
      || std::is_assignable_v<T&, const optional<U>&>
      || std::is_assignable_v<T&, optional<U>&&>
      || std::is_assignable_v<T&, const optional<U>&&>;

public:
  using value_type = T;

  constexpr optional() noexcept = default;

  constexpr optional(nullopt_t) noexcept {};

  constexpr optional(const optional& other)
    requires(!is_reference_v && !std::is_trivially_copy_constructible_v<T>
             && std::is_copy_constructible_v<T>)
      : base_type()
  {
    if (other) {
      std::construct_at(std::addressof(this->val_), other.value());
      this->has_ = true;
    }
  }

  constexpr optional(const optional& other)
    requires(is_reference_v && !std::is_trivially_copy_constructible_v<T>
             && std::is_copy_constructible_v<T>)
  {
    if (other) {
      this->valptr_ = std::addressof(other.value());
      this->has_ = true;
    }
  }

  constexpr optional(const optional&)
    requires(is_reference_v || std::is_trivially_copy_constructible_v<T>)
  = default;

  constexpr optional(optional&& other) noexcept(
      std::is_nothrow_move_constructible_v<T>)
    requires(!is_reference_v && !std::is_trivially_move_constructible_v<T>
             && std::is_move_constructible_v<T>)
  {
    if (other) {
      std::construct_at(std::addressof(this->val_), std::move(other.value()));
      this->has_ = true;
    }
  }

  constexpr optional(optional&&)
    requires(is_reference_v || std::is_trivially_move_constructible_v<T>)
  = default;

  template <typename U>
  constexpr explicit(!std::is_convertible_v<const U&, T>)
      optional(const optional<U>& other)
    requires(!is_reference_v && std::is_constructible_v<T, const U&>
             && !is_optional_like_construct_v<U>)
  {
    if (other) {
      std::construct_at(std::addressof(this->val_), other.value());
      this->has_ = true;
    }
  }

  template <typename U>
  constexpr explicit(!std::is_convertible_v<U&&, T>)
      optional(optional<U>&& other)
    requires(!is_reference_v && std::is_constructible_v<T, U &&>
             && !is_optional_like_construct_v<U>)
  {
    if (other) {
      std::construct_at(std::addressof(this->val_), std::move(other.value()));
      this->has_ = true;
    }
  }

  template <typename... Args>
  constexpr explicit optional(std::in_place_t, Args&&... args)
    requires(!is_reference_v && std::is_constructible_v<T, Args...>)
  {
    std::construct_at(std::addressof(this->val_), std::forward<Args>(args)...);
    this->has_ = true;
  }

  template <typename U, typename... Args>
  constexpr explicit optional(std::in_place_t,
                              std::initializer_list<U> ilist,
                              Args&&... args)
    requires(
        !is_reference_v
        && std::is_constructible_v<T, std::initializer_list<U>&, Args && ...>)
  {
    std::construct_at(std::addressof(this->val_), ilist,
                      std::forward<Args>(args)...);
    this->has_ = true;
  }

  template <typename U = T>
  constexpr explicit(!std::is_convertible_v<U&&, T>) optional(U&& value)
    requires(!is_reference_v && std::is_constructible_v<T, U &&>
             && !std::is_same_v<std::remove_cvref_t<U>, std::in_place_t>
             && !std::is_same_v<std::remove_cvref_t<U>, optional>)
      : base_type(std::forward<U>(value))
  {
  }

  template <typename U = T>
  constexpr explicit(!std::is_convertible_v<U&&, T>) optional(U& value)
    requires(is_reference_v
             && !std::is_same_v<std::remove_cvref_t<U>, std::in_place_t>
             && !std::is_same_v<std::remove_cvref_t<U>, optional>)
      : base_type(std::in_place, value)
  {
  }

  constexpr ~optional() = default;

  constexpr optional& operator=(nullopt_t) noexcept
    requires(!is_reference_v)
  {
    if (has_value()) {
      std::destroy_at(std::addressof(this->val_));
      this->has = false;
    }
  }

  constexpr optional& operator=(nullopt_t) noexcept
    requires(is_reference_v)
  {
    if (has_value()) {
      this->valptr_ = nullptr;
      this->has = false;
    }
  }

  constexpr optional& operator=(const optional&)
    requires(is_reference_v
             || std::is_trivially_copy_constructible_v<T>
                 && std::is_trivially_copy_assignable_v<T>)
  = default;

  constexpr optional& operator=(const optional& other)
    requires(!is_reference_v && !std::is_trivially_copy_constructible_v<T>
             && !std::is_trivially_copy_assignable_v<T>
             && std::is_copy_constructible_v<T> && std::is_copy_assignable_v<T>)
  {
    if (has_value()) {
      if (other.has_value()) {
        this->val_ = other.value();
      } else {
        std::destroy_at(std::addressof(this->val_));
        this->has_ = false;
      }
    } else {
      if (other.has_value()) {
        std::construct_at(std::addressof(this->val_), other.value());
        this->has_ = true;
      }
    }
  }

  constexpr optional& operator=(optional&&)
    requires(is_reference_v
             || std::is_trivially_move_constructible_v<T>
                 && std::is_trivially_move_assignable_v<T>)
  = default;

  constexpr optional& operator=(optional&& other)
    requires(!is_reference_v && !std::is_trivially_move_constructible_v<T>
             && !std::is_trivially_move_assignable_v<T>
             && std::is_move_constructible_v<T> && std::is_move_assignable_v<T>)
  {
    if (has_value()) {
      if (other.has_value()) {
        this->val_ = std::move(other.value());
      } else {
        std::destroy_at(std::addressof(this->val_));
        this->has_ = false;
      }
    } else {
      if (other.has_value()) {
        std::construct_at(std::addressof(this->val_), std::move(other.value()));
        this->has_ = true;
      }
    }
  }

  template <typename U = T>
  constexpr optional& operator=(U&& value)
    requires(!is_reference_v
             && std::is_constructible_v<T, U> && std::is_assignable_v<T&, U>
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
  }

  template <typename U>
  constexpr optional& operator=(const optional<U>& other)
    requires(!is_reference_v && std::is_constructible_v<T, const U&>
             && std::is_assignable_v<T&, const U&>
             && !is_optional_like_assign_v<U>)
  {
    if (has_value()) {
      this->val_ = other.value();
    } else {
      std::construct_at(std::addressof(this->val_), other.value());
      this->has_ = true;
    }
  }

  template <typename U>
  constexpr optional& operator=(optional<U>&& other)
    requires(!is_reference_v && std::is_constructible_v<T, U>
             && std::is_assignable_v<T&, U> && !is_optional_like_assign_v<U>)
  {
    if (has_value()) {
      this->val_ = std::move(other.value());
    } else {
      std::construct_at(std::addressof(this->val_), std::move(other.value()));
      this->has_ = true;
    }
  }

  constexpr std::add_pointer_t<T> operator->() noexcept
    requires(!is_reference_v)
  {
    FITORIA_ASSERT(has_value());
    return std::addressof(this->val_);
  }

  constexpr std::add_pointer_t<const T> operator->() const noexcept
    requires(!is_reference_v)
  {
    FITORIA_ASSERT(has_value());
    return std::addressof(this->val_);
  }

  constexpr std::add_pointer_t<T> operator->() noexcept
    requires(is_reference_v)
  {
    FITORIA_ASSERT(has_value());
    return this->valptr_;
  }

  constexpr std::add_pointer_t<const T> operator->() const noexcept
    requires(is_reference_v)
  {
    FITORIA_ASSERT(has_value());
    return this->valptr_;
  }

  constexpr T& operator*() & noexcept
    requires(!is_reference_v)
  {
    FITORIA_ASSERT(has_value());
    return this->val_;
  }

  constexpr const T& operator*() const& noexcept
    requires(!is_reference_v)
  {
    FITORIA_ASSERT(has_value());
    return this->val_;
  }

  constexpr T&& operator*() && noexcept
    requires(!is_reference_v)
  {
    FITORIA_ASSERT(has_value());
    return std::move(this->val_);
  }

  constexpr const T&& operator*() const&& noexcept
    requires(!is_reference_v)
  {
    FITORIA_ASSERT(has_value());
    return std::move(this->val_);
  }

  constexpr T& operator*() const& noexcept
    requires(is_reference_v)
  {
    FITORIA_ASSERT(has_value());
    return *this->valptr_;
  }

  constexpr T&& operator*() const&& noexcept
    requires(is_reference_v)
  {
    FITORIA_ASSERT(has_value());
    return std::move(*this->valptr_);
  }

  constexpr explicit operator bool() const noexcept
  {
    return has_value();
  }

  constexpr bool has_value() const noexcept
    requires(!is_reference_v)
  {
    return this->has_;
  }

  constexpr bool has_value() const noexcept
    requires(is_reference_v)
  {
    return this->valptr_ != nullptr;
  }

  constexpr T& value() &
        requires(!is_reference_v)
  {
    if (!has_value()) {
      throw bad_optional_access();
    }

    return this->val_;
  }

  constexpr const T& value() const&
    requires(!is_reference_v)
  {
    if (!has_value()) {
      throw bad_optional_access();
    }

    return this->val_;
  }

  constexpr T&& value() &&
        requires(!is_reference_v)
  {
    if (!has_value()) {
      throw bad_optional_access();
    }

    return std::move(this->val_);
  }

  constexpr const T&& value() const&&
    requires(!is_reference_v)
  {
    if (!has_value()) {
      throw bad_optional_access();
    }

    return std::move(this->val_);
  }

  constexpr T& value() const&
    requires(is_reference_v)
  {
    if (!has_value()) {
      throw bad_optional_access();
    }

    return *this->valptr_;
  }

  constexpr T&& value() const&&
    requires(is_reference_v)
  {
    if (!has_value()) {
      throw bad_optional_access();
    }

    return std::move(*this->valptr_);
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
    static_assert(
        is_specialization_of_v<std::invoke_result_t<F, T&>, optional>);
    if (has_value()) {
      return std::invoke(std::forward<F>(f), value());
    } else {
      return std::remove_cvref_t<std::invoke_result_t<F, T&>>();
    }
  }

  template <typename F>
  constexpr auto and_then(F&& f) const&
  {
    static_assert(
        is_specialization_of_v<std::invoke_result_t<F, const T&>, optional>);
    if (has_value()) {
      return std::invoke(std::forward<F>(f), value());
    } else {
      return std::remove_cvref_t<std::invoke_result_t<F, const T&>>();
    }
  }

  template <typename F>
  constexpr auto and_then(F&& f) &&
  {
    static_assert(is_specialization_of_v<std::invoke_result_t<F, T>, optional>);
    if (has_value()) {
      return std::invoke(std::forward<F>(f), std::move(value()));
    } else {
      return std::remove_cvref_t<std::invoke_result_t<F, T>>();
    }
  }

  template <typename F>
  constexpr auto and_then(F&& f) const&&
  {
    static_assert(
        is_specialization_of_v<std::invoke_result_t<F, const T>, optional>);
    if (has_value()) {
      return std::invoke(std::forward<F>(f), std::move(value()));
    } else {
      return std::remove_cvref_t<std::invoke_result_t<F, const T>>();
    }
  }

  template <typename F>
  constexpr auto transform(F&& f) &
  {
    using U = std::remove_cv_t<std::invoke_result_t<F, T&>>;
    if (has_value()) {
      return optional<U>(std::invoke(std::forward<F>(f), value()));
    } else {
      return optional<U>();
    }
  }

  template <typename F>
  constexpr auto transform(F&& f) const&
  {
    using U = std::remove_cv_t<std::invoke_result_t<F, const T&>>;
    if (has_value()) {
      return optional<U>(std::invoke(std::forward<F>(f), value()));
    } else {
      return optional<U>();
    }
  }

  template <typename F>
  constexpr auto transform(F&& f) &&
  {
    using U = std::remove_cv_t<std::invoke_result_t<F, T>>;
    if (has_value()) {
      return optional<U>(std::invoke(std::forward<F>(f), std::move(value())));
    } else {
      return optional<U>();
    }
  }

  template <typename F>
  constexpr auto transform(F&& f) const&&
  {
    using U = std::remove_cv_t<std::invoke_result_t<F, const T>>;
    if (has_value()) {
      return optional<U>(std::invoke(std::forward<F>(f), std::move(value())));
    } else {
      return optional<U>();
    }
  }

  template <typename F>
  constexpr optional or_else(F&& f) const&
    requires(std::is_copy_constructible_v<T> && std::is_invocable_v<F>)
  {
    using U = std::remove_cvref_t<std::invoke_result_t<F>>;
    static_assert(std::is_same_v<U, optional>);
    if (has_value()) {
      return value();
    } else {
      return std::forward<F>(f)();
    }
  }

  template <typename F>
  constexpr optional or_else(F&& f) &&
    requires(std::is_move_constructible_v<T>&& std::is_invocable_v<F>)
  {
    using U = std::remove_cvref_t<std::invoke_result_t<F>>;
    static_assert(std::is_same_v<U, optional>);
    if (has_value()) {
      return std::move(value());
    } else {
      return std::forward<F>(f)();
    }
  }

  template <typename E>
  constexpr auto to_expected_or(E&& default_error) const&
  {
    using Exp = expected<T, std::remove_cvref_t<E>>;
    if (has_value()) {
      return Exp(value());
    } else {
      return Exp(unexpect, std::forward<E>(default_error));
    }
  }

  template <typename E>
  constexpr auto to_expected_or(E&& default_error) &&
  {
    using Exp = expected<T, std::remove_cvref_t<E>>;
    if (has_value()) {
      return Exp(std::move(value()));
    } else {
      return Exp(unexpect, std::forward<E>(default_error));
    }
  }
  constexpr void swap(optional& other)
    requires(!is_reference_v && std::is_move_constructible_v<T>)
  {
    if (has_value()) {
      if (other.has_value()) {
        using std::swap;
        swap(this->val_, other.value());
      } else {
        std::construct_at(std::addressof(other->val_), std::move(value()));
        std::destroy_at(std::addressof(this->val_));
        this->has_ = false;
        other->has_ = true;
      }
    } else {
      if (other.has_value()) {
        other.swap(*this);
      }
    }
  }

  constexpr void reset() noexcept
    requires(!is_reference_v && !std::is_trivially_destructible_v<T>)
  {
    if (has_value()) {
      this->val_.~T();
    }
  }

  constexpr void reset() noexcept
    requires(!is_reference_v && std::is_trivially_destructible_v<T>)
  {
  }

  constexpr void reset() noexcept
    requires(is_reference_v)
  {
    this->valptr_ = nullptr;
  }

  template <typename... Args>
  constexpr T& emplace(Args&&... args)
    requires(!is_reference_v)
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
    requires(!is_reference_v)
  {
    if (has_value()) {
      std::destroy_at(std::addressof(this->val_));
      this->has_ = false;
    }

    std::construct_at(std::addressof(this->val_), ilist,
                      std::forward<Args>(args)...);
    this->has_ = true;
    return this->val_;
  }
};

template <typename T1, typename T2>
constexpr bool operator==(const optional<T1>& lhs, const optional<T2>& rhs)
{
  if (lhs.has_value() != rhs.has_value()) {
    return false;
  }

  if (!lhs) {
    return true;
  }

  return lhs.value() == rhs.value();
}

template <typename T1, typename T2>
constexpr bool operator!=(const optional<T1>& lhs, const optional<T2>& rhs)
{
  if (lhs.has_value() != rhs.has_value()) {
    return true;
  }

  if (!lhs) {
    return false;
  }

  return lhs.value() != rhs.value();
}

template <typename T1, typename T2>
constexpr bool operator<(const optional<T1>& lhs, const optional<T2>& rhs)
{
  if (!rhs) {
    return false;
  }
  if (!lhs) {
    return true;
  }

  return lhs.value() < rhs.value();
}

template <typename T1, typename T2>
constexpr bool operator<=(const optional<T1>& lhs, const optional<T2>& rhs)
{
  if (!lhs) {
    return true;
  }
  if (!rhs) {
    return false;
  }

  return lhs.value() <= rhs.value();
}

template <typename T1, typename T2>
constexpr bool operator>(const optional<T1>& lhs, const optional<T2>& rhs)
{
  if (!lhs) {
    return false;
  }
  if (!rhs) {
    return true;
  }

  return lhs.value() > rhs.value();
}

template <typename T1, typename T2>
constexpr bool operator>=(const optional<T1>& lhs, const optional<T2>& rhs)
{
  if (!rhs) {
    return true;
  }
  if (!lhs) {
    return false;
  }

  return lhs.value() >= rhs.value();
}

template <typename T, std::three_way_comparable_with<T> U>
constexpr std::compare_three_way_result_t<T, U>
operator<=>(const optional<T>& lhs, const optional<U>& rhs)
{
  return lhs.value() <=> rhs.value();
}

template <typename T>
constexpr bool operator==(const optional<T>& opt, nullopt_t) noexcept
{
  return opt;
}

template <typename T>
constexpr std::strong_ordering operator<=>(const optional<T>& opt,
                                           nullopt_t) noexcept
{
  return bool(opt) <=> false;
}

template <typename T, typename U>
constexpr bool operator==(const optional<T>& opt, const U& val)
{
  return opt && opt.value() == val;
}

template <typename T, typename U>
constexpr bool operator==(const T& val, const optional<U>& opt)
{
  return opt && val == opt.value();
}

template <typename T, typename U>
constexpr bool operator!=(const optional<T>& opt, const U& val)
{
  return !opt || opt.value() != val;
}

template <typename T, typename U>
constexpr bool operator!=(const T& val, const optional<U>& opt)
{
  return !opt || val != opt.value();
}

template <typename T, typename U>
constexpr bool operator<(const optional<T>& opt, const U& val)
{
  return !opt || opt.value() < val;
}

template <typename T, typename U>
constexpr bool operator<(const T& val, const optional<U>& opt)
{
  return opt && val < opt.value();
}

template <typename T, typename U>
constexpr bool operator<=(const optional<T>& opt, const U& val)
{
  return !opt || opt.value() <= val;
}

template <typename T, typename U>
constexpr bool operator<=(const T& val, const optional<U>& opt)
{
  return opt && val <= opt.value();
}

template <typename T, typename U>
constexpr bool operator>(const optional<T>& opt, const U& val)
{
  return !(opt <= val);
}

template <typename T, typename U>
constexpr bool operator>(const T& val, const optional<U>& opt)
{
  return !(val <= opt);
}

template <typename T, typename U>
constexpr bool operator>=(const optional<T>& opt, const U& val)
{
  return !(opt < val);
}

template <typename T, typename U>
constexpr bool operator>=(const T& val, const optional<U>& opt)
{
  return !(val < opt);
}

template <typename T, std::three_way_comparable_with<T> U>
constexpr std::compare_three_way_result_t<T, U>
operator<=>(const optional<T>& opt, const U& value)
{
  return bool(opt) ? *opt <=> value : std::strong_ordering::less;
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
  return optional<T>(std::in_place, std::move(ilist),
                     std::forward<Args>(args)...);
}

template <typename T>
constexpr void swap(optional<T>& lhs,
                    optional<T>& rhs) noexcept(noexcept(lhs.swap(rhs)))
  requires(std::is_move_constructible_v<T> && std::is_swappable_v<T>)
{
  lhs.swap(rhs);
}

FITORIA_NAMESPACE_END
