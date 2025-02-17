#pragma once

#include <map>
#include <vector>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <deque>

// #include "web_server_container.h"
#include "web_handler.h"

namespace web_server
{
  class HandlerStaticUri : public HandlerBase // public Controller, public Component,
  {
  protected:
    const char *_uri;

  public:
    HandlerStaticUri(const char *uri) : _uri(uri) {};
    HandlerStaticUri(IHandlerContainer &handlerContainer, const char *uri) : HandlerBase(handlerContainer), _uri(uri) {};
  };

  /// @brief For static text
  class HandlerStaticUriText : public HandlerStaticUri
  {
  private:
    const char *_text;
    const ssize_t _text_len;


  protected:
    bool canHandle(AsyncWebServerRequest *request) override;
    void handleRequest(AsyncWebServerRequest *request) override;

  public:
    HandlerStaticUriText(const char *uri);
    HandlerStaticUriText(const char *uri, const char *text);
    HandlerStaticUriText(const char *uri, const char *text, ssize_t text_len);
    HandlerStaticUriText(IHandlerContainer &handlerContainer, const char *uri);
    HandlerStaticUriText(IHandlerContainer &handlerContainer, const char *uri, const char *text);
    HandlerStaticUriText(IHandlerContainer &handlerContainer, const char *uri, const char *text, ssize_t text_len);
  };

  /// @brief For binary and may be gziped
  class HandlerStaticUriBin : public HandlerStaticUri
  {
  private:
    const uint8_t *_buf;
    const ssize_t _buf_len;
    const bool _is_gzip{false};

  protected:
    /// Override the web handler's canHandle method.
    bool canHandle(AsyncWebServerRequest *request) override;
    /// Override the web handler's handleRequest method.
    void handleRequest(AsyncWebServerRequest *request) override;

  public:
    HandlerStaticUriBin(const char *uri, const uint8_t *buf, ssize_t buf_len, bool is_gzip = false);
    HandlerStaticUriBin(IHandlerContainer &handlerContainer, const char *uri, const uint8_t *buf, ssize_t buf_len, bool is_gzip = false);
  };

} // namespace web_server
