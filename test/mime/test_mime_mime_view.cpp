//
// Copyright (c) 2024 Ramirisu (labyrinth.ramirisu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <fitoria/test/test.hpp>

#include <fitoria/mime/mime_view.hpp>

using namespace fitoria;
using namespace fitoria::mime;

TEST_SUITE_BEGIN("[fitoria.mime.mime_view]");

TEST_CASE("contants")
{
  {
    auto m = application_javascript();
    CHECK_EQ(m.source(), "application/javascript");
    CHECK_EQ(m.type(), "application");
    CHECK_EQ(m.subtype(), "javascript");
    CHECK_EQ(m.suffix(), "");
  }
  {
    auto m = application_json();
    CHECK_EQ(m.source(), "application/json");
    CHECK_EQ(m.type(), "application");
    CHECK_EQ(m.subtype(), "json");
    CHECK_EQ(m.suffix(), "");
  }
  {
    auto m = application_octet_stream();
    CHECK_EQ(m.source(), "application/octet-stream");
    CHECK_EQ(m.type(), "application");
    CHECK_EQ(m.subtype(), "octet-stream");
    CHECK_EQ(m.suffix(), "");
  }
  {
    auto m = application_pdf();
    CHECK_EQ(m.source(), "application/pdf");
    CHECK_EQ(m.type(), "application");
    CHECK_EQ(m.subtype(), "pdf");
    CHECK_EQ(m.suffix(), "");
  }
  {
    auto m = application_www_form_urlencoded();
    CHECK_EQ(m.source(), "application/x-www-form-urlencoded");
    CHECK_EQ(m.type(), "application");
    CHECK_EQ(m.subtype(), "x-www-form-urlencoded");
    CHECK_EQ(m.suffix(), "");
  }
  {
    auto m = image_bmp();
    CHECK_EQ(m.source(), "image/bmp");
    CHECK_EQ(m.type(), "image");
    CHECK_EQ(m.subtype(), "bmp");
    CHECK_EQ(m.suffix(), "");
  }
  {
    auto m = image_gif();
    CHECK_EQ(m.source(), "image/gif");
    CHECK_EQ(m.type(), "image");
    CHECK_EQ(m.subtype(), "gif");
    CHECK_EQ(m.suffix(), "");
  }
  {
    auto m = image_jpeg();
    CHECK_EQ(m.source(), "image/jpeg");
    CHECK_EQ(m.type(), "image");
    CHECK_EQ(m.subtype(), "jpeg");
    CHECK_EQ(m.suffix(), "");
  }
  {
    auto m = image_png();
    CHECK_EQ(m.source(), "image/png");
    CHECK_EQ(m.type(), "image");
    CHECK_EQ(m.subtype(), "png");
    CHECK_EQ(m.suffix(), "");
  }
  {
    auto m = image_svg();
    CHECK_EQ(m.source(), "image/svg+xml");
    CHECK_EQ(m.type(), "image");
    CHECK_EQ(m.subtype(), "svg");
    CHECK_EQ(m.suffix(), "xml");
  }
  {
    auto m = text_html();
    CHECK_EQ(m.source(), "text/html");
    CHECK_EQ(m.type(), "text");
    CHECK_EQ(m.subtype(), "html");
    CHECK_EQ(m.suffix(), "");
  }
  {
    auto m = text_javascript();
    CHECK_EQ(m.source(), "text/javascript");
    CHECK_EQ(m.type(), "text");
    CHECK_EQ(m.subtype(), "javascript");
    CHECK_EQ(m.suffix(), "");
  }
  {
    auto m = text_plain();
    CHECK_EQ(m.source(), "text/plain");
    CHECK_EQ(m.type(), "text");
    CHECK_EQ(m.subtype(), "plain");
    CHECK_EQ(m.suffix(), "");
  }
  {
    auto m = text_xml();
    CHECK_EQ(m.source(), "text/xml");
    CHECK_EQ(m.type(), "text");
    CHECK_EQ(m.subtype(), "xml");
    CHECK_EQ(m.suffix(), "");
  }
}

TEST_CASE("compare")
{
  CHECK_EQ(mime_view::parse("text/plain"), mime_view::parse("TEXT/PLAIN"));
  CHECK_EQ(mime_view::parse("image/svg+xml"),
           mime_view::parse("IMAGE/SVG+XML"));

  CHECK_EQ(mime_view::parse("text/plain"), "TEXT/PLAIN");
  CHECK_EQ(mime_view::parse("image/svg+xml"), "IMAGE/SVG+XML");
}

TEST_SUITE_END();
