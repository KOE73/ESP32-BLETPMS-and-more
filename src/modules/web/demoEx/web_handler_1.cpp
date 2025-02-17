// #include "web_server.h"
#include "web_handler_1.h"

#include "json_util.h"
// #include "esphome/components/network/util.h"
// #include "esphome/core/application.h"
// #include "esphome/core/entity_base.h"
#include "helpers.h"
#include "esp_log.h"
// #include "esphome/core/util.h"

#include "web_url_match.hpp"

#include <cstdlib>

namespace web_server
{

  static const char *const TAG = "web_server";

#pragma region AsyncWebHandler_1

  HandlerStaticUriText::HandlerStaticUriText(const char *uri)
      : HandlerStaticUriText(uri, "oK") {}

  HandlerStaticUriText::HandlerStaticUriText(const char *uri, const char *text)
      : HandlerStaticUriText(uri, text, strlen(text)) {}

  HandlerStaticUriText::HandlerStaticUriText(const char *uri, const char *text, ssize_t text_len)
      : HandlerStaticUri(uri), _text(text), _text_len(text_len)
  {
  }

  HandlerStaticUriText::HandlerStaticUriText(IHandlerContainer &handlerContainer, const char *uri)
      : HandlerStaticUriText(handlerContainer, uri, "oK") {}

  HandlerStaticUriText::HandlerStaticUriText(IHandlerContainer &handlerContainer, const char *uri, const char *text)
      : HandlerStaticUriText(handlerContainer, uri, text, strlen(text)) {}

  HandlerStaticUriText::HandlerStaticUriText(IHandlerContainer &handlerContainer, const char *uri, const char *text, ssize_t text_len)
      : HandlerStaticUri(handlerContainer, uri), _text(text), _text_len(text_len)
  {
  }

  bool HandlerStaticUriText::canHandle(AsyncWebServerRequest *request)
  {

    if (request->url() == _uri)
    {
      ESP_LOGI(TAG, "AsyncWebHandler_1::canHandle %s", request->url().c_str());
      return true;
    }

    return false;
  }

  void HandlerStaticUriText::handleRequest(AsyncWebServerRequest *request)
  {
    ESP_LOGI(TAG, "AsyncWebHandler_1::handleRequest %s", request->url().c_str());

    // "text/css" ....
    // text/javascript
    if (request->url() == _uri)
    {
      AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html", reinterpret_cast<const uint8_t *>(_text), _text_len);
      // response->addHeader("Content-Encoding", "gzip");
      request->send(response);
      return;
    }
  }

#pragma endregion

#pragma region AsyncWebHandler_2

  HandlerStaticUriBin::HandlerStaticUriBin(const char *uri, const uint8_t *buf, ssize_t buf_len, bool is_gzip)
      : HandlerStaticUri(uri), _buf(buf), _buf_len(buf_len), _is_gzip(is_gzip)
  {
  }

  HandlerStaticUriBin::HandlerStaticUriBin(IHandlerContainer &handlerContainer, const char *uri, const uint8_t *buf, ssize_t buf_len, bool is_gzip)
      : HandlerStaticUri(handlerContainer,uri), _buf(buf), _buf_len(buf_len), _is_gzip(is_gzip)
  {
  }

  bool HandlerStaticUriBin::canHandle(AsyncWebServerRequest *request)
  {

    if (request->url() == _uri)
    {
      ESP_LOGI(TAG, "AsyncWebHandler_2::canHandle %s", request->url().c_str());
      return true;
    }

    return false;
  }

  void HandlerStaticUriBin::handleRequest(AsyncWebServerRequest *request)
  {
    ESP_LOGI(TAG, "AsyncWebHandler_2::handleRequest %s", request->url().c_str());

    if (request->url() == _uri)
    {
      AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html", reinterpret_cast<const uint8_t *>(_buf), _buf_len);
      if (_is_gzip)
        response->addHeader("Content-Encoding", "gzip");
      request->send(response);
      return;
    }
  }

#pragma endregion

} // namespace web_server
