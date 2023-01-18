//
// Copyright (c) 2022-2023 Ramirisu (labyrinth dot ramirisu at gmail dot com)
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

namespace cacert {

#if defined(FITORIA_HAS_OPENSSL)
inline void add_builtin_cacerts(net::ssl::context& ssl_ctx)
{
  // clang-format off
  std::string_view cacerts_p1 = 
  // Amazon Root CA 1
  // ================
  "-----BEGIN CERTIFICATE-----\n"
  "MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ikPmljZbyjANBgkqhkiG9w0BAQsFADA5MQswCQYD\n"
  "VQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6b24gUm9vdCBDQSAxMB4XDTE1\n"
  "MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTELMAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpv\n"
  "bjEZMBcGA1UEAxMQQW1hem9uIFJvb3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoC\n"
  "ggEBALJ4gHHKeNXjca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgH\n"
  "FzZM9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qwIFAGbHrQ\n"
  "gLKm+a/sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6VOujw5H5SNz/0egwLX0t\n"
  "dHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L93FcXmn/6pUCyziKrlA4b9v7LWIbxcce\n"
  "VOF34GfID5yHI9Y/QCB/IIDEgEw+OyQmjgSubJrIqg0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB\n"
  "/zAOBgNVHQ8BAf8EBAMCAYYwHQYDVR0OBBYEFIQYzIU07LwMlJQuCFmcx7IQTgoIMA0GCSqGSIb3\n"
  "DQEBCwUAA4IBAQCY8jdaQZChGsV2USggNiMOruYou6r4lK5IpDB/G/wkjUu0yKGX9rbxenDIU5PM\n"
  "CCjjmCXPI6T53iHTfIUJrU6adTrCC2qJeHZERxhlbI1Bjjt/msv0tadQ1wUsN+gDS63pYaACbvXy\n"
  "8MWy7Vu33PqUXHeeE6V/Uq2V8viTO96LXFvKWlJbYK8U90vvo/ufQJVtMVT8QtPHRh8jrdkPSHCa\n"
  "2XV4cdFyQzR1bldZwgJcJmApzyMZFo6IQ6XU5MsI+yMRQ+hDKXJioaldXgjUkK642M4UwtBV8ob2\n"
  "xJNDd2ZhwLnoQdeXeGADbkpyrqXRfboQnoZsG4q5WTP468SQvvG5\n"
  "-----END CERTIFICATE-----\n"
  "\n"
  // Amazon Root CA 2
  // ================
  "-----BEGIN CERTIFICATE-----\n"
  "MIIFQTCCAymgAwIBAgITBmyf0pY1hp8KD+WGePhbJruKNzANBgkqhkiG9w0BAQwFADA5MQswCQYD\n"
  "VQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6b24gUm9vdCBDQSAyMB4XDTE1\n"
  "MDUyNjAwMDAwMFoXDTQwMDUyNjAwMDAwMFowOTELMAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpv\n"
  "bjEZMBcGA1UEAxMQQW1hem9uIFJvb3QgQ0EgMjCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoC\n"
  "ggIBAK2Wny2cSkxKgXlRmeyKy2tgURO8TW0G/LAIjd0ZEGrHJgw12MBvIITplLGbhQPDW9tK6Mj4\n"
  "kHbZW0/jTOgGNk3Mmqw9DJArktQGGWCsN0R5hYGCrVo34A3MnaZMUnbqQ523BNFQ9lXg1dKmSYXp\n"
  "N+nKfq5clU1Imj+uIFptiJXZNLhSGkOQsL9sBbm2eLfq0OQ6PBJTYv9K8nu+NQWpEjTj82R0Yiw9\n"
  "AElaKP4yRLuH3WUnAnE72kr3H9rN9yFVkE8P7K6C4Z9r2UXTu/Bfh+08LDmG2j/e7HJV63mjrdvd\n"
  "fLC6HM783k81ds8P+HgfajZRRidhW+mez/CiVX18JYpvL7TFz4QuK/0NURBs+18bvBt+xa47mAEx\n"
  "kv8LV/SasrlX6avvDXbR8O70zoan4G7ptGmh32n2M8ZpLpcTnqWHsFcQgTfJU7O7f/aS0ZzQGPSS\n"
  "btqDT6ZjmUyl+17vIWR6IF9sZIUVyzfpYgwLKhbcAS4y2j5L9Z469hdAlO+ekQiG+r5jqFoz7Mt0\n"
  "Q5X5bGlSNscpb/xVA1wf+5+9R+vnSUeVC06JIglJ4PVhHvG/LopyboBZ/1c6+XUyo05f7O0oYtlN\n"
  "c/LMgRdg7c3r3NunysV+Ar3yVAhU/bQtCSwXVEqY0VThUWcI0u1ufm8/0i2BWSlmy5A5lREedCf+\n"
  "3euvAgMBAAGjQjBAMA8GA1UdEwEB/wQFMAMBAf8wDgYDVR0PAQH/BAQDAgGGMB0GA1UdDgQWBBSw\n"
  "DPBMMPQFWAJI/TPlUq9LhONmUjANBgkqhkiG9w0BAQwFAAOCAgEAqqiAjw54o+Ci1M3m9Zh6O+oA\n"
  "A7CXDpO8Wqj2LIxyh6mx/H9z/WNxeKWHWc8w4Q0QshNabYL1auaAn6AFC2jkR2vHat+2/XcycuUY\n"
  "+gn0oJMsXdKMdYV2ZZAMA3m3MSNjrXiDCYZohMr/+c8mmpJ5581LxedhpxfL86kSk5Nrp+gvU5LE\n"
  "YFiwzAJRGFuFjWJZY7attN6a+yb3ACfAXVU3dJnJUH/jWS5E4ywl7uxMMne0nxrpS10gxdr9HIcW\n"
  "xkPo1LsmmkVwXqkLN1PiRnsn/eBG8om3zEK2yygmbtmlyTrIQRNg91CMFa6ybRoVGld45pIq2WWQ\n"
  "gj9sAq+uEjonljYE1x2igGOpm/HlurR8FLBOybEfdF849lHqm/osohHUqS0nGkWxr7JOcQ3AWEbW\n"
  "aQbLU8uz/mtBzUF+fUwPfHJ5elnNXkoOrJupmHN5fLT0zLm4BwyydFy4x2+IoZCn9Kr5v2c69BoV\n"
  "Yh63n749sSmvZ6ES8lgQGVMDMBu4Gon2nL2XA46jCfMdiyHxtN/kHNGfZQIG6lzWE7OE76KlXIx3\n"
  "KadowGuuQNKotOrN8I1LOJwZmhsoVLiJkO/KdYE+HvJkJMcYr07/R54H9jVlpNMKVv/1F2Rs76gi\n"
  "JUmTtt8AF9pYfl3uxRuw0dFfIRDH+fO6AgonB8Xx1sfT4PsJYGw=\n"
  "-----END CERTIFICATE-----\n"
  "\n"
  // Amazon Root CA 3
  // ================
  "-----BEGIN CERTIFICATE-----\n"
  "MIIBtjCCAVugAwIBAgITBmyf1XSXNmY/Owua2eiedgPySjAKBggqhkjOPQQDAjA5MQswCQYDVQQG\n"
  "EwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6b24gUm9vdCBDQSAzMB4XDTE1MDUy\n"
  "NjAwMDAwMFoXDTQwMDUyNjAwMDAwMFowOTELMAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZ\n"
  "MBcGA1UEAxMQQW1hem9uIFJvb3QgQ0EgMzBZMBMGByqGSM49AgEGCCqGSM49AwEHA0IABCmXp8ZB\n"
  "f8ANm+gBG1bG8lKlui2yEujSLtf6ycXYqm0fc4E7O5hrOXwzpcVOho6AF2hiRVd9RFgdszflZwjr\n"
  "Zt6jQjBAMA8GA1UdEwEB/wQFMAMBAf8wDgYDVR0PAQH/BAQDAgGGMB0GA1UdDgQWBBSrttvXBp43\n"
  "rDCGB5Fwx5zEGbF4wDAKBggqhkjOPQQDAgNJADBGAiEA4IWSoxe3jfkrBqWTrBqYaGFy+uGh0Psc\n"
  "eGCmQ5nFuMQCIQCcAu/xlJyzlvnrxir4tiz+OpAUFteMYyRIHN8wfdVoOw==\n"
  "-----END CERTIFICATE-----\n"
  "\n"
  // Amazon Root CA 4
  // ================
  "-----BEGIN CERTIFICATE-----\n"
  "MIIB8jCCAXigAwIBAgITBmyf18G7EEwpQ+Vxe3ssyBrBDjAKBggqhkjOPQQDAzA5MQswCQYDVQQG\n"
  "EwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6b24gUm9vdCBDQSA0MB4XDTE1MDUy\n"
  "NjAwMDAwMFoXDTQwMDUyNjAwMDAwMFowOTELMAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZ\n"
  "MBcGA1UEAxMQQW1hem9uIFJvb3QgQ0EgNDB2MBAGByqGSM49AgEGBSuBBAAiA2IABNKrijdPo1MN\n"
  "/sGKe0uoe0ZLY7Bi9i0b2whxIdIA6GO9mif78DluXeo9pcmBqqNbIJhFXRbb/egQbeOc4OO9X4Ri\n"
  "83BkM6DLJC9wuoihKqB1+IGuYgbEgds5bimwHvouXKNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNV\n"
  "HQ8BAf8EBAMCAYYwHQYDVR0OBBYEFNPsxzplbszh2naaVvuc84ZtV+WBMAoGCCqGSM49BAMDA2gA\n"
  "MGUCMDqLIfG9fhGt0O9Yli/W651+kI0rz2ZVwyzjKKlwCkcO8DdZEv8tmZQoTipPNU0zWgIxAOp1\n"
  "AE47xDqUEpHJWEadIRNyp4iciuRMStuW1KyLa2tJElMzrdfkviT8tQp21KW8EA==\n"
  "-----END CERTIFICATE-----\n"
  "\n";
  // clang-format on

  ssl_ctx.add_certificate_authority(
      net::buffer(cacerts_p1.data(), cacerts_p1.size()));
}

#endif

}

FITORIA_NAMESPACE_END
