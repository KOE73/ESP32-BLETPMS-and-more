#pragma once

#include <map>
#include <vector>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <deque>

// #include "web_server_container.h"
#include "handler.h"

namespace yaidfws
{

  /// @brief For static text
  class HandlerStaticUriText : public HandlerStaticUrl
  {
  private:
    const char *_text;
    const ssize_t _text_len;

  protected:
    void handleRequest(AsyncWebServerRequest *request) override;

  public:
    HandlerStaticUriText(std::string url);
    HandlerStaticUriText(std::string url, const char *text);
    HandlerStaticUriText(std::string url, const char *text, ssize_t text_len);
    HandlerStaticUriText(IHandlerContainer &handlerContainer, std::string url);
    HandlerStaticUriText(IHandlerContainer &handlerContainer, std::string url, const char *text);
    HandlerStaticUriText(IHandlerContainer &handlerContainer, std::string url, const char *text, ssize_t text_len);
  };

  /// @brief For binary and may be gziped
  class HandlerStaticUriBin : public HandlerStaticUrl
  {
  private:
    const uint8_t *_buf;
    const ssize_t _buf_len;
    const bool _is_gzip{false};

  protected:
  
    void handleRequest(AsyncWebServerRequest *request) override;

  public:
    HandlerStaticUriBin(std::string url, const uint8_t *buf, ssize_t buf_len, bool is_gzip = false);
    HandlerStaticUriBin(IHandlerContainer &handlerContainer, std::string url, const uint8_t *buf, ssize_t buf_len, bool is_gzip = false);
  };

} // namespace yaidfws
