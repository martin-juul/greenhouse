#pragma once
#include "TCPSocket.h"
#include "socket.h"
#include "database.h"
#include "TLSSocket.h"

static const int MAX_CONN = 5;

static const char cert[] = \
  "-----BEGIN CERTIFICATE-----\n"
  "MIID3zCCAsegAwIBAgIUMftUP+WG7Y3BBMd+HmqYMgpmDM8wDQYJKoZIhvcNAQEL\n"
  "BQAwfzELMAkGA1UEBhMCREsxEzARBgNVBAgMClN5ZGRhbm1hcmsxDzANBgNVBAcM\n"
  "Bk9kZW5zZTETMBEGA1UECgwKR3JlZW5ob3VzZTEWMBQGA1UEAwwNMTkyLjE2OC4x\n"
  "LjEwMDEdMBsGCSqGSIb3DQEJARYObG9jYWxob3N0QHJvb3QwHhcNMjEwODA1MDgz\n"
  "MjM0WhcNMjEwOTA0MDgzMjM0WjB/MQswCQYDVQQGEwJESzETMBEGA1UECAwKU3lk\n"
  "ZGFubWFyazEPMA0GA1UEBwwGT2RlbnNlMRMwEQYDVQQKDApHcmVlbmhvdXNlMRYw\n"
  "FAYDVQQDDA0xOTIuMTY4LjEuMTAwMR0wGwYJKoZIhvcNAQkBFg5sb2NhbGhvc3RA\n"
  "cm9vdDCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAJJf1sS7HnUY/MZ6\n"
  "t8O0BJ8G1mpMbqTK5ZFxTCsu7ReujVi+ppiBufPYRf+dIFslpS0YOVJ+4Ep/vTaK\n"
  "62F/tbfc62AcJ2LV+NGGekg7advYvStsE08XQntTf6eKF0djzkEq6jePvibuVcvX\n"
  "cWSKjBe0wP3K94Nm9/Kb7zraVlAGhUgm5lgZG62g9Uk5TNaFTjN6SDXDauiKMHND\n"
  "b8V3k9OKON51OshMpl7CIZUzXoITW6k1kfj06AKKv2i8A3Mf08ImGXllRH0Mioer\n"
  "VkMtgGaFC9E+vj1Xhafhsjm21bIbYX1UXR3sAtSiu9QGqAIGpgTET/F4F8u8xmQs\n"
  "vH3uCsECAwEAAaNTMFEwHQYDVR0OBBYEFNwuKckENZtjlw+PA4b2lCOEbfhPMB8G\n"
  "A1UdIwQYMBaAFNwuKckENZtjlw+PA4b2lCOEbfhPMA8GA1UdEwEB/wQFMAMBAf8w\n"
  "DQYJKoZIhvcNAQELBQADggEBACMdgHCXeZ/Pp5a+Oq8p9SMetCNTbV5B+TiqUD8w\n"
  "d8T9jVqCw5lQ20gCWg4Om7rsZMs2XN5OrWj7ExiY1C3bSMCjiTdyIbjTTysAoodS\n"
  "wlX0r/O9RtJeNrlSMFFvxlJqmiRlAQhE4Sa5Ql+2E2kcXEf7CCxW+ksTpdFgKonh\n"
  "vigGj9KHiE6Bko6Gu2VS8PEWshT94sOQYxLqQUWSvSztyhDDlNqi4ZCEgek1+0K5\n"
  "9RkovM3Wib6U1Ulbr6edPE0saPGCy2icA8OE5pQXHrqjLH58OtZcydkia/KtVdMv\n"
  "/MWZJ2H9oTi1ftEIauqM0rqrPKeiYDzNuAsecYwRtI4K22o=\n"
  "-----END CERTIFICATE-----";

static const char private_key[] = \
  "-----BEGIN PRIVATE KEY-----\n"
  "MIIEvAIBADANBgkqhkiG9w0BAQEFAASCBKYwggSiAgEAAoIBAQCSX9bEux51GPzG\n"
  "erfDtASfBtZqTG6kyuWRcUwrLu0Xro1YvqaYgbnz2EX/nSBbJaUtGDlSfuBKf702\n"
  "iuthf7W33OtgHCdi1fjRhnpIO2nb2L0rbBNPF0J7U3+nihdHY85BKuo3j74m7lXL\n"
  "13FkiowXtMD9yveDZvfym+862lZQBoVIJuZYGRutoPVJOUzWhU4zekg1w2roijBz\n"
  "Q2/Fd5PTijjedTrITKZewiGVM16CE1upNZH49OgCir9ovANzH9PCJhl5ZUR9DIqH\n"
  "q1ZDLYBmhQvRPr49V4Wn4bI5ttWyG2F9VF0d7ALUorvUBqgCBqYExE/xeBfLvMZk\n"
  "LLx97grBAgMBAAECggEAbfgM7c67KjR0rFVEPIbBtgw4CLKVqb85zBR3qjXeMJ2W\n"
  "WecaWJeV8go5RLpytafJueBqKH1j2vSvXgvunViKWLFkMNfWC15ASoIi2qOwe1gR\n"
  "f076DsCl1Ou9i0ynAp6MNc+dGdMnhC4pTTT2Xxfr+l91YvcbD6DLhXz7tZaVpBav\n"
  "K66rl15LwXEHV2JjhnCTgMyvWs4QgBBRUrKRjtWPXjFAXrwcVu+JKwlRASPi35Xo\n"
  "HWZZpBx0/1B/X5dIjEo6A7bl+j7YzAa3yCV/qswAp685dVzAO+Ds3K925aP4M0Cw\n"
  "Y7G0u79O7GhHG4RkJWk1QPhpPsKUO6zlY9e8lCtbZQKBgQDB5Q77AuRT7pvNaIZP\n"
  "7xcl1Ewufe8Gs6oG0qREnMxbf3T3Ui3w2R1cSdtkvGYSLT78T7/tXj+YYP1OdIOy\n"
  "21uATdBoTTUSKrel+d9mVy87FGc5U58PrKyTO80+RJJMCeowZhnuO+RK0GNwhVPz\n"
  "IZKSF+tbmYPbFJoZIps3TKveAwKBgQDBQjawA2p5JNWz14S+1HkU8fI1XEWy+IB3\n"
  "eiMEcppU3nHXzxu6c/oAM4M02xBxQTAHLBZxXMdDieCYjpHGIvcAnRZu6/4vEpsV\n"
  "vDRZ8f64hN4ueXPqeqMnjRQTWRsgs7GUuptPbK+cy1g3ll5QBV3uLVIxDNm4dq20\n"
  "89fC3hRq6wKBgGzdrJyZUhQiLA6X+pnU951A5wlPoi6UIAo2qQ7xLvjMxqK63JqX\n"
  "fRm9Slb0RSc9iljmei/PtEtdYbx0RSHecjDHABYgC4VIwNaLRrjSiO1WFmMkpXGK\n"
  "lVcRw7qLi8zv4DMOkM6niBXNjZe29JzVo9MtZPOIbXVLyv4D5TyWawV7AoGAadcN\n"
  "hFAam8Ykk53QwYIz3dizwa2LLn0SPlzRmLn688fvqTVwBilGHZ4YmXXwB1agKw1m\n"
  "LSeWqdJ1ApfyFgVIKTne81ijmLwj2z97nFcAHaYG2IUDisMnX53QBNikFsDS3JVm\n"
  "QZVQiBqFccwUUtbdnneLr/lNs8yrTZLN85cUr4kCgYAWpsOvptW9hLcbIFrI1lXH\n"
  "K1bZh/6ojZ9T7RtxuM2uK0+XuCYB3j+o3NkMeHG8snzrNX0WwWWZVVccYZrYyjuA\n"
  "SbgLd+Ou7IY9Xh5cuDJQkc7Du86QlT6MqfGJgy85M5JlB2rmiW81hl0z1qQeCoke\n"
  "YjR9+Dyk0G5qcPmZK6Gn2Q==\n"
  "-----END PRIVATE KEY-----";

class WebServer {
public:
  WebServer(Database *database);
  /** Starts the webserver
   *
   * @retval void
   */
  int start();
  /** One "revolution" of parsing incoming/outgoing connections
   *
   * @retval void
   */
  void tick();
  /** Get the underlying socket.
   *
   * Use this socket to close the server.
   *
   * @retval Socket
   */
  TCPSocket *getSocket();
};