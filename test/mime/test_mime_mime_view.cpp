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

TEST_CASE("parse")
{
  {
    auto m = mime_view::parse("text/plain");
    CHECK_EQ(m->source(), "text/plain");
    CHECK_EQ(m->essence(), "text/plain");
    CHECK_EQ(m->type(), "text");
    CHECK_EQ(m->subtype(), "plain");
    CHECK_EQ(m->suffix(), nullopt);
    CHECK_EQ(m->params(), params_view {});
  }
  {
    auto m = mime_view::parse("text/plain; charset=utf-8");
    CHECK_EQ(m->source(), "text/plain; charset=utf-8");
    CHECK_EQ(m->essence(), "text/plain");
    CHECK_EQ(m->type(), "text");
    CHECK_EQ(m->subtype(), "plain");
    CHECK_EQ(m->suffix(), nullopt);
    CHECK_EQ(m->params(), params_view { { "charset", "utf-8" } });
  }
  {
    auto m = mime_view::parse("text/plain; charset=utf-8; author=fitoria");
    CHECK_EQ(m->source(), "text/plain; charset=utf-8; author=fitoria");
    CHECK_EQ(m->essence(), "text/plain");
    CHECK_EQ(m->type(), "text");
    CHECK_EQ(m->subtype(), "plain");
    CHECK_EQ(m->suffix(), nullopt);
    CHECK_EQ(m->params(),
             params_view { { "charset", "utf-8" }, { "author", "fitoria" } });
  }
  {
    auto m = mime_view::parse("image/svg+xml");
    CHECK_EQ(m->source(), "image/svg+xml");
    CHECK_EQ(m->essence(), "image/svg+xml");
    CHECK_EQ(m->type(), "image");
    CHECK_EQ(m->subtype(), "svg");
    CHECK_EQ(m->suffix(), "xml");
  }
  {
    CHECK(!mime_view::parse("text/"));
    CHECK(!mime_view::parse("/plain"));
    CHECK(!mime_view::parse("text/plain+"));
    CHECK(!mime_view::parse("image/svg/xml"));
    CHECK(!mime_view::parse("image+svg+xml"));
    CHECK(!mime_view::parse("-text/plain"));
    CHECK(!mime_view::parse("text/-plain"));
    CHECK(!mime_view::parse("text\\plain"));
    CHECK(!mime_view::parse("i@mage/svg+xml"));
    CHECK(!mime_view::parse("image/s@vg+xml"));
    CHECK(!mime_view::parse("image/svg+x@ml"));
    CHECK(!mime_view::parse("text/plain; charset"));
    CHECK(!mime_view::parse("text/plain; charset="));
    CHECK(!mime_view::parse("text/plain; charset=utf-8=utf-16"));
    CHECK(!mime_view::parse("text/plain; ="));
    CHECK(!mime_view::parse("text/plain; =utf-8"));
    CHECK(!mime_view::parse("text/plain; charset=utf:8"));
  }
}

TEST_CASE("from_extension")
{
  CHECK_EQ(mime_view::from_extension("js"), application_javascript());
  CHECK_EQ(mime_view::from_extension("json"), application_json());
  CHECK_EQ(mime_view::from_extension("swf"), application_shockwave_flash());
  CHECK_EQ(mime_view::from_extension("xml"), application_xml());
  CHECK_EQ(mime_view::from_extension("bmp"), image_bmp());
  CHECK_EQ(mime_view::from_extension("gif"), image_gif());
  CHECK_EQ(mime_view::from_extension("jpe"), image_jpeg());
  CHECK_EQ(mime_view::from_extension("jpeg"), image_jpeg());
  CHECK_EQ(mime_view::from_extension("jpg"), image_jpeg());
  CHECK_EQ(mime_view::from_extension("png"), image_png());
  CHECK_EQ(mime_view::from_extension("svg"), image_svg());
  CHECK_EQ(mime_view::from_extension("svgz"), image_svg());
  CHECK_EQ(mime_view::from_extension("tif"), image_tiff());
  CHECK_EQ(mime_view::from_extension("tiff"), image_tiff());
  CHECK_EQ(mime_view::from_extension("css"), text_css());
  CHECK_EQ(mime_view::from_extension("htm"), text_html());
  CHECK_EQ(mime_view::from_extension("html"), text_html());
  CHECK_EQ(mime_view::from_extension("php"), text_html());
  CHECK_EQ(mime_view::from_extension("txt"), text_plain());
  CHECK_EQ(mime_view::from_extension("flv"), video_flv());

  CHECK_EQ(mime_view::from_extension(""), nullopt);
  CHECK_EQ(mime_view::from_extension("abc"), nullopt);
}

TEST_CASE("from_path")
{
  CHECK_EQ(mime_view::from_path("abc.js"), application_javascript());
  CHECK_EQ(mime_view::from_path("abc.cde.js"), application_javascript());

  CHECK_EQ(mime_view::from_path("abc"), nullopt);
  CHECK_EQ(mime_view::from_path("abc."), nullopt);
  CHECK_EQ(mime_view::from_path("."), nullopt);
}

TEST_CASE("contants")
{
  {
    auto m = application_javascript();
    CHECK_EQ(m.essence(), "application/javascript");
    CHECK_EQ(m.type(), "application");
    CHECK_EQ(m.subtype(), "javascript");
    CHECK_EQ(m.suffix(), nullopt);
    CHECK_EQ(m.params(), params_view {});
  }
  {
    auto m = application_json();
    CHECK_EQ(m.essence(), "application/json");
    CHECK_EQ(m.type(), "application");
    CHECK_EQ(m.subtype(), "json");
    CHECK_EQ(m.suffix(), nullopt);
    CHECK_EQ(m.params(), params_view {});
  }
  {
    auto m = application_msgpack();
    CHECK_EQ(m.essence(), "application/msgpack");
    CHECK_EQ(m.type(), "application");
    CHECK_EQ(m.subtype(), "msgpack");
    CHECK_EQ(m.suffix(), nullopt);
    CHECK_EQ(m.params(), params_view {});
  }
  {
    auto m = application_octet_stream();
    CHECK_EQ(m.essence(), "application/octet-stream");
    CHECK_EQ(m.type(), "application");
    CHECK_EQ(m.subtype(), "octet-stream");
    CHECK_EQ(m.suffix(), nullopt);
    CHECK_EQ(m.params(), params_view {});
  }
  {
    auto m = application_pdf();
    CHECK_EQ(m.essence(), "application/pdf");
    CHECK_EQ(m.type(), "application");
    CHECK_EQ(m.subtype(), "pdf");
    CHECK_EQ(m.suffix(), nullopt);
    CHECK_EQ(m.params(), params_view {});
  }
  {
    auto m = application_shockwave_flash();
    CHECK_EQ(m.essence(), "application/x-shockwave-flash");
    CHECK_EQ(m.type(), "application");
    CHECK_EQ(m.subtype(), "x-shockwave-flash");
    CHECK_EQ(m.suffix(), nullopt);
    CHECK_EQ(m.params(), params_view {});
  }
  {
    auto m = application_text();
    CHECK_EQ(m.essence(), "application/text");
    CHECK_EQ(m.type(), "application");
    CHECK_EQ(m.subtype(), "text");
    CHECK_EQ(m.suffix(), nullopt);
    CHECK_EQ(m.params(), params_view {});
  }
  {
    auto m = application_www_form_urlencoded();
    CHECK_EQ(m.essence(), "application/x-www-form-urlencoded");
    CHECK_EQ(m.type(), "application");
    CHECK_EQ(m.subtype(), "x-www-form-urlencoded");
    CHECK_EQ(m.suffix(), nullopt);
    CHECK_EQ(m.params(), params_view {});
  }
  {
    auto m = application_xml();
    CHECK_EQ(m.essence(), "application/xml");
    CHECK_EQ(m.type(), "application");
    CHECK_EQ(m.subtype(), "xml");
    CHECK_EQ(m.suffix(), nullopt);
    CHECK_EQ(m.params(), params_view {});
  }
  {
    auto m = image_bmp();
    CHECK_EQ(m.essence(), "image/bmp");
    CHECK_EQ(m.type(), "image");
    CHECK_EQ(m.subtype(), "bmp");
    CHECK_EQ(m.suffix(), nullopt);
    CHECK_EQ(m.params(), params_view {});
  }
  {
    auto m = image_gif();
    CHECK_EQ(m.essence(), "image/gif");
    CHECK_EQ(m.type(), "image");
    CHECK_EQ(m.subtype(), "gif");
    CHECK_EQ(m.suffix(), nullopt);
    CHECK_EQ(m.params(), params_view {});
  }
  {
    auto m = image_jpeg();
    CHECK_EQ(m.essence(), "image/jpeg");
    CHECK_EQ(m.type(), "image");
    CHECK_EQ(m.subtype(), "jpeg");
    CHECK_EQ(m.suffix(), nullopt);
    CHECK_EQ(m.params(), params_view {});
  }
  {
    auto m = image_png();
    CHECK_EQ(m.essence(), "image/png");
    CHECK_EQ(m.type(), "image");
    CHECK_EQ(m.subtype(), "png");
    CHECK_EQ(m.suffix(), nullopt);
    CHECK_EQ(m.params(), params_view {});
  }
  {
    auto m = image_svg();
    CHECK_EQ(m.essence(), "image/svg+xml");
    CHECK_EQ(m.type(), "image");
    CHECK_EQ(m.subtype(), "svg");
    CHECK_EQ(m.suffix(), "xml");
    CHECK_EQ(m.params(), params_view {});
  }
  {
    auto m = image_tiff();
    CHECK_EQ(m.essence(), "image/tiff");
    CHECK_EQ(m.type(), "image");
    CHECK_EQ(m.subtype(), "tiff");
    CHECK_EQ(m.suffix(), nullopt);
    CHECK_EQ(m.params(), params_view {});
  }
  {
    auto m = multipart_form_data();
    CHECK_EQ(m.essence(), "multipart/form-data");
    CHECK_EQ(m.type(), "multipart");
    CHECK_EQ(m.subtype(), "form-data");
    CHECK_EQ(m.suffix(), nullopt);
    CHECK_EQ(m.params(), params_view {});
  }
  {
    auto m = text_css();
    CHECK_EQ(m.essence(), "text/css");
    CHECK_EQ(m.type(), "text");
    CHECK_EQ(m.subtype(), "css");
    CHECK_EQ(m.suffix(), nullopt);
    CHECK_EQ(m.params(), params_view {});
  }
  {
    auto m = text_csv();
    CHECK_EQ(m.essence(), "text/csv");
    CHECK_EQ(m.type(), "text");
    CHECK_EQ(m.subtype(), "csv");
    CHECK_EQ(m.suffix(), nullopt);
    CHECK_EQ(m.params(), params_view {});
  }
  {
    auto m = text_html();
    CHECK_EQ(m.essence(), "text/html");
    CHECK_EQ(m.type(), "text");
    CHECK_EQ(m.subtype(), "html");
    CHECK_EQ(m.suffix(), nullopt);
    CHECK_EQ(m.params(), params_view {});
  }
  {
    auto m = text_javascript();
    CHECK_EQ(m.essence(), "text/javascript");
    CHECK_EQ(m.type(), "text");
    CHECK_EQ(m.subtype(), "javascript");
    CHECK_EQ(m.suffix(), nullopt);
    CHECK_EQ(m.params(), params_view {});
  }
  {
    auto m = text_plain();
    CHECK_EQ(m.essence(), "text/plain");
    CHECK_EQ(m.type(), "text");
    CHECK_EQ(m.subtype(), "plain");
    CHECK_EQ(m.suffix(), nullopt);
    CHECK_EQ(m.params(), params_view {});
  }
  {
    auto m = text_xml();
    CHECK_EQ(m.essence(), "text/xml");
    CHECK_EQ(m.type(), "text");
    CHECK_EQ(m.subtype(), "xml");
    CHECK_EQ(m.suffix(), nullopt);
    CHECK_EQ(m.params(), params_view {});
  }
  {
    auto m = video_flv();
    CHECK_EQ(m.essence(), "video/x-flv");
    CHECK_EQ(m.type(), "video");
    CHECK_EQ(m.subtype(), "x-flv");
    CHECK_EQ(m.suffix(), nullopt);
    CHECK_EQ(m.params(), params_view {});
  }
}

TEST_CASE("compare")
{
  CHECK_EQ(mime_view::parse("text/plain"), mime_view::parse("TEXT/PLAIN"));
  CHECK_EQ(mime_view::parse("image/svg+xml"),
           mime_view::parse("IMAGE/SVG+XML"));

  CHECK_EQ(mime_view::parse("text/plain"), "TEXT/PLAIN");
  CHECK_EQ(mime_view::parse("image/svg+xml"), "IMAGE/SVG+XML");

  {
    auto m = mime_view::parse("image/svg+xml; charset=utf-8");
    CHECK_EQ(m->essence(), "IMAGE/SVG+XML");
    CHECK_EQ(m->type(), "IMAGE");
    CHECK_EQ(m->subtype(), "SVG");
    CHECK_EQ(m->suffix(), "XML");
    CHECK_EQ(m->params(), params_view { { "CHARSET", "UTF-8" } });
  }
}

TEST_CASE("format")
{
  CHECK_EQ(fmt::format("{}", mime::application_json()), "application/json");
}

TEST_SUITE_END();
