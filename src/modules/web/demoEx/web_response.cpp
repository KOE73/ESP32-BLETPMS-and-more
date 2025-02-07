

#include <cstdarg>

// #include "esphome/core/log.h"
// #include "esphome/core/helpers.h"

#include "esp_log.h"

#include "esp_tls_crypto.h"

#include "utils.h"
#include "web_server_idf.h"
#include "web_response.h"


static const char *TAG_WEB2_SERVER = "WEB2_SERVER";
#define LOG_WEB2_COLOR LOG_ANSI_COLOR_BOLD_BACKGROUND(LOG_COLOR_BLUE, LOG_ANSI_COLOR_BG_CYAN)

namespace esphome
{
  namespace web_server_idf
  {

#ifndef HTTPD_409
#define HTTPD_409 "409 Conflict"
#endif

#define CRLF_STR "\r\n"
#define CRLF_LEN (sizeof(CRLF_STR) - 1)

    static const char *const TAG = "web_server_idf";


   
    void AsyncWebServerResponse::addHeader(const char *name, const char *value)
    {
      httpd_resp_set_hdr(*this->req_, name, value);
    }

    void AsyncResponseStream::print(float value) { this->print(to_string(value)); }

    void AsyncResponseStream::printf(const char *fmt, ...)
    {
      va_list args;

      va_start(args, fmt);
      const int length = vsnprintf(nullptr, 0, fmt, args);
      va_end(args);

      std::string str;
      str.resize(length);

      va_start(args, fmt);
      vsnprintf(&str[0], length + 1, fmt, args);
      va_end(args);

      this->print(str);
    }

  } // namespace web_server_idf
} // namespace esphome
