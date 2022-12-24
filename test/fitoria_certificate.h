//
// Copyright (c) 2022 Ramirisu (labyrinth dot ramirisu at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include <fitoria/core/config.hpp>

#include <fitoria/core/net.hpp>

FITORIA_NAMESPACE_BEGIN

namespace cert {

using namespace fitoria::net;

#if defined(FITORIA_HAS_OPENSSL)
static const std::string key
    = "-----BEGIN PRIVATE KEY-----\n"
      "MIIEvgIBADANBgkqhkiG9w0BAQEFAASCBKgwggSkAgEAAoIBAQDEc3UP0Vcfp4pR\n"
      "J4JxLAcTyutWgbqaLup9TOc20i18DHcL5//mRkIAaNjsYuzIW2rvrcNy7I2cK6UQ\n"
      "jrZssu8vuNjyBFpfMQbsTTDK09QcNCfp96V88HZ1fQnEMlfaAlbrGcOGbV/rQyH6\n"
      "/eLbxA9+4MlzwAYlzub4enTCJGnBboSK3X5vEWW6UNaq8lqj1DxSTZhv6ULqxy6O\n"
      "B6p657a+GgUSlAq5BOwS1O3UK6BfhrGPH3IaOuTaRwHULa5OP/mkmTJk9TDA3dp7\n"
      "fsnPI+1rZGArGO6301TdID0GwM9TsWTnRUtuSRovNYVU7bPtgxrBGOnKWqa0QzbG\n"
      "cO/9vTjzAgMBAAECggEAUbRXVonRe3fZ/iC5jsop82Bs+qCf9GpY2l42t8U4Z6Rj\n"
      "is8lzicgYj05Iy+NnVLwKdkSNeUOjIpWb4l7c8AD3Mw7ZoTK9VZVs8fy6Bir2kMS\n"
      "6qs157229uxXOINbqG9aqRMhD/LDdvyH0HTxekeTzkg9YOfXZrgjtepwTyKH3YuX\n"
      "tFAALyLw70BXD7aLKxkA+R8lCCeb6UGOGte8ufHmv5zX7rlPSzJ4UlXwyyoZ7oc8\n"
      "a2W3EAU9xfqDt3Afzlue7Gl3F0i1VhZS7L0AHz4yoPd8UzaMM7TqE44n6kBPjw4x\n"
      "0XPxdT9ixbpdbhxvyak3dTzID/BqJjtnH3ZcmInEWQKBgQDt7dzCETGjAf1odAtm\n"
      "u71WEQonhOqjQF25EmkUchO5OMPHzLMzgkEzlqN/Y4yFHnWovuHI2RvDjliBbRVX\n"
      "HOrYzhZuLJYED6U1YRwpfmNutlx0Ov5xDrSACNest019GlteVhkMwbMAdE0zYvda\n"
      "6NPAaCnTERmn0uo+wgPh6O8P9wKBgQDTXx8HrjZ3uPF1ciRIv6eosEuoYBk2SatL\n"
      "RWjse3FzkxJp+7wi4ZSftex7oBvet14doTakFPKh30WdsnMCbLSMl848jc3V7h0D\n"
      "aIv/6kgDt15V38DIfo3urmTDgx08MjNLf7cWvLuX8JnKmczRH5MQFt4m6xCk9FLh\n"
      "IUru2oFX5QKBgQDgJd98oNAfXmeKeRCopyOXKpRjaK/YPnJNQY/nKBV2wIiI+Uq1\n"
      "sveUD01WXMVsP6be4304R3kFgVvWCps5J1AEEhtFjhGCH+8nW6Y5XQ11pq1w/obo\n"
      "oc7gc/QjjcidnDAJrpkI0I6/VdHJIGohpgifUkJcytZdqbDukL8xm81kPwKBgB57\n"
      "SjNGWU58mj2Qn1Dqapg2DEMU0K32g8BQGyh3SM4c+HvWNLRnCZLanmbZmQtOMEFf\n"
      "nUyeRLs5QoXSVSmVWZgo0DIFVRizxPVX31ChjY8To4QIx3liXr974MYdI1sp9iG+\n"
      "Ij4iK39m0BbahSHnRsL8OT8tiPHw1f7nWTIGyr5NAoGBAMh1QmJ8r6ypmkhDVD4u\n"
      "65DP2F62RqV7CE0jAmzuvr4BWGmISZlrkgufZS1CFNrbmki3vxJ9jXFbf++1IZOf\n"
      "9une2dw9xRtb7DfWk7hj9juphmrFOfjlAWBppm0XLAo0sEWnsoP8jY59DQZL+Pe2\n"
      "L4A+7H0uIXotBcvK7kuK0nwY\n"
      "-----END PRIVATE KEY-----\n";

static const std::string cert
    = "-----BEGIN CERTIFICATE-----\n"
      "MIIDmDCCAoCgAwIBAgIUC/F8EaviczpG6+rQZ+QdP+RDOMEwDQYJKoZIhvcNAQEL\n"
      "BQAwSjEUMBIGA1UEAwwLZml0b3JpYS5jb20xCzAJBgNVBAYTAlRXMRQwEgYDVQQI\n"
      "DAtUYWlwZWkgQ2l0eTEPMA0GA1UEBwwGVGFpcGVpMCAXDTIyMTIyMjE3NDUyOVoY\n"
      "DzIwNTIxMjE0MTc0NTI5WjBKMRQwEgYDVQQDDAtmaXRvcmlhLmNvbTELMAkGA1UE\n"
      "BhMCVFcxFDASBgNVBAgMC1RhaXBlaSBDaXR5MQ8wDQYDVQQHDAZUYWlwZWkwggEi\n"
      "MA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQDEc3UP0Vcfp4pRJ4JxLAcTyutW\n"
      "gbqaLup9TOc20i18DHcL5//mRkIAaNjsYuzIW2rvrcNy7I2cK6UQjrZssu8vuNjy\n"
      "BFpfMQbsTTDK09QcNCfp96V88HZ1fQnEMlfaAlbrGcOGbV/rQyH6/eLbxA9+4Mlz\n"
      "wAYlzub4enTCJGnBboSK3X5vEWW6UNaq8lqj1DxSTZhv6ULqxy6OB6p657a+GgUS\n"
      "lAq5BOwS1O3UK6BfhrGPH3IaOuTaRwHULa5OP/mkmTJk9TDA3dp7fsnPI+1rZGAr\n"
      "GO6301TdID0GwM9TsWTnRUtuSRovNYVU7bPtgxrBGOnKWqa0QzbGcO/9vTjzAgMB\n"
      "AAGjdDByMB0GA1UdDgQWBBROHBqHOS6AtSO+m+ZPcebm0sOnEDAfBgNVHSMEGDAW\n"
      "gBROHBqHOS6AtSO+m+ZPcebm0sOnEDAOBgNVHQ8BAf8EBAMCBaAwIAYDVR0lAQH/\n"
      "BBYwFAYIKwYBBQUHAwEGCCsGAQUFBwMCMA0GCSqGSIb3DQEBCwUAA4IBAQBZmFEO\n"
      "aU+sPJ1a4WOKD6zf2kqH1Y6B/K9db3b6I+GU+aF65/8npdJT5RFs8bkpZw53TJb2\n"
      "a1avH7cjx/ocSmBrQL3HuLKHaNsPU4PMMTxgiQaFgejrDgjY64QGObT6HCew2mZ9\n"
      "KTn4mOtGN+GBhxoGtX0JeRjV1lIzVr0vdXMsgVv7F7Mbb3+soRrZHEQtVYIdsI/5\n"
      "dzfE9BZmTa1qa5PiDVDUPv64EsPUgkBHSNfAx6BdUDzydCiVNglHJ+zyK+m4Md8V\n"
      "FVUg8EmC0KA6lW9IO0IYvGc2YfporprxFUe1Snkl+fGrs0wnhPdceYdWhGPCSoZ8\n"
      "dvAf5JDZXjiI6LaZ\n"
      "-----END CERTIFICATE-----\n";

static const std::string dh
    = "-----BEGIN DH PARAMETERS-----\n"
      "MIIBCAKCAQEArzQc5mpm0Fs8yahDeySj31JZlwEphUdZ9StM2D8+Fo7TMduGtSi+\n"
      "/HRWVwHcTFAgrxVdm+dl474mOUqqaz4MpzIb6+6OVfWHbQJmXPepZKyu4LgUPvY/\n"
      "4q3/iDMjIS0fLOu/bLuObwU5ccZmDgfhmz1GanRlTQOiYRty3FiOATWZBRh6uv4u\n"
      "tff4A9Bm3V9tLx9S6djq31w31Gl7OQhryodW28kc16t9TvO1BzcV3HjRPwpe701X\n"
      "oEEZdnZWANkkpR/m/pfgdmGPU66S2sXMHgsliViQWpDCYeehrvFRHEdR9NV+XJfC\n"
      "QMUk26jPTIVTLfXmmwU0u8vUkpR7LQKkwwIBAg==\n"
      "-----END DH PARAMETERS-----\n";

inline net::ssl::context get_server_ssl_ctx(net::ssl::context::method ssl_ver)
{

  net::ssl::context ssl_ctx(ssl_ver);
  ssl_ctx.set_options(net::ssl::context::default_workarounds
                      | net::ssl::context::single_dh_use);

  ssl_ctx.use_certificate_chain(net::buffer(cert.data(), cert.size()));

  ssl_ctx.use_private_key(net::buffer(key.data(), key.size()),
                          net::ssl::context::file_format::pem);

  ssl_ctx.use_tmp_dh(net::buffer(dh.data(), dh.size()));

  return ssl_ctx;
}

inline net::ssl::context get_client_ssl_ctx(net::ssl::context::method ssl_ver)
{
  net::ssl::context ssl_ctx(ssl_ver);
  ssl_ctx.set_verify_mode(net::ssl::verify_peer);
  ssl_ctx.add_certificate_authority(net::buffer(cert.data(), cert.size()));
  return ssl_ctx;
}
#endif

}

FITORIA_NAMESPACE_END
