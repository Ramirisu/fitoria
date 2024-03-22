//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#ifndef FITORIA_LOG_FORMATTER_HPP
#define FITORIA_LOG_FORMATTER_HPP

#include <fitoria/core/config.hpp>

#include <fitoria/core/optional.hpp>

#include <fitoria/log/record.hpp>

#include <array>

FITORIA_NAMESPACE_BEGIN

namespace log {

enum class newline_style {
  lf,
  crlf,
  cr,
#if defined(FITORIA_TARGET_WINDOWS)
  system = crlf,
#else
  system = lf,
#endif
};

inline auto get_newline(newline_style style) -> std::string_view
{
  switch (style) {
  case newline_style::lf:
    return "\n";
  case newline_style::crlf:
    return "\r\n";
  case newline_style::cr:
    return "\r";
  default:
    break;
  }

  return "\n";
}

enum class file_name_style {
  full_path,
  file_name_only,
};

inline std::string_view get_file_name(file_name_style style,
                                      std::string_view file_path) noexcept
{
  if (style == file_name_style::file_name_only) {
    if (auto pos = file_path.find_last_of("\\/");
        pos != std::string_view::npos) {
      return file_path.substr(pos + 1);
    }
  }

  return file_path;
}

using level_style_formatter = std::function<fmt::text_style(level)>;

class formatter {
public:
  using format_string = fmt::format_string<
      fmt::detail::named_arg<char, record::clock_t::time_point>,
      fmt::detail::named_arg<char, level>,
      fmt::detail::named_arg<char, std::string>, // msg
      fmt::detail::named_arg<char, std::uint32_t>, // line
      fmt::detail::named_arg<char, std::uint32_t>, // column
      fmt::detail::named_arg<char, std::string_view>, // file
      fmt::detail::named_arg<char, std::string_view> // function
      >;

  class builder {
    friend class formatter;

  public:
    builder() = default;

    auto build() const -> formatter
    {
      return formatter(*this);
    }

    builder& set_pattern(format_string fmt)
    {
      fmt_ = fmt;
      return *this;
    }

    builder& set_newline_style(newline_style style)
    {
      nl_style_ = style;
      return *this;
    }

    builder& set_file_name_style(file_name_style style)
    {
      fn_style_ = style;
      return *this;
    }

    builder& set_level_style(level_style_formatter style)
    {
      level_style_.emplace(style);
      return *this;
    }

    builder& set_color_level_style()
    {
      level_style_ = [](level lv) -> fmt::text_style {
        static const auto colors
            = std::array<fmt::color, static_cast<std::size_t>(level::count)> {
                fmt::color::white,  fmt::color::cyan, fmt::color::lime_green,
                fmt::color::yellow, fmt::color::red,  fmt::color::magenta,
                fmt::color::black
              };
        return fmt::fg(colors[static_cast<std::size_t>(lv)]);
      };
      return *this;
    }

  private:
    format_string fmt_
        = { "[{TIME:%FT%TZ} {LV:} {FUNC:}] {MSG:} [{FILE:}:{LINE:}:{COL:}]" };
    newline_style nl_style_ = newline_style::system;
    file_name_style fn_style_ = file_name_style::file_name_only;
    optional<level_style_formatter> level_style_;
  };

  formatter()
      : formatter(builder().build())
  {
  }

  formatter(builder builder)
      : fmt_(builder.fmt_)
      , nl_style_(builder.nl_style_)
      , fn_style_(builder.fn_style_)
      , level_style_(builder.level_style_)
  {
  }

  auto format(record_ptr rec) const -> std::string
  {
    auto s = fmt::format(
        fmt::runtime(fmt_),
        fmt::arg("TIME", rec->time),
        fmt::arg("LV", format(rec->lv)),
        fmt::arg("MSG", rec->msg),
        fmt::arg("LINE", rec->line),
        fmt::arg("COL", rec->column),
        fmt::arg("FILE", get_file_name(fn_style_, rec->file_name)),
        fmt::arg("FUNC", rec->function_name));
    s += get_newline(nl_style_);
    return s;
  }

private:
  auto format(level lv) const -> std::string
  {
    if (level_style_) {
      return fmt::format("{}", fmt::styled(lv, level_style_->operator()(lv)));
    }

    return to_string(lv);
  }

  format_string fmt_;
  newline_style nl_style_;
  file_name_style fn_style_;
  optional<level_style_formatter> level_style_;
};

}

FITORIA_NAMESPACE_END

#endif
