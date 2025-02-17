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

  AsyncWebHandler_1::AsyncWebHandler_1(const char *uri) : AsyncWebHandler_1(uri, "oK") {}

  AsyncWebHandler_1::AsyncWebHandler_1(const char *uri, const char *text) : AsyncWebHandler_1(uri, text, strlen(text)) {}

  AsyncWebHandler_1::AsyncWebHandler_1(const char *uri, const char *text, ssize_t buf_len) : _uri(uri) //, _method(HTTP_GET)
  {
    _text = text;
    _text_len = buf_len;
  }

  bool AsyncWebHandler_1::canHandle(AsyncWebServerRequest *request)
  {

    if (request->url() == _uri)
    {
      ESP_LOGI(TAG, "AsyncWebHandler_1::canHandle %s", request->url().c_str());
      return true;
    }

    return false;
  }

  void AsyncWebHandler_1::handleRequest(AsyncWebServerRequest *request)
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

  AsyncWebHandler_2::AsyncWebHandler_2(const char *uri, const uint8_t *buf, ssize_t buf_len, bool is_gzip)
      : _uri(uri), _buf(buf), _buf_len(buf_len), _is_gzip(is_gzip)
  {
  }

  bool AsyncWebHandler_2::canHandle(AsyncWebServerRequest *request)
  {

    if (request->url() == _uri)
    {
      ESP_LOGI(TAG, "AsyncWebHandler_2::canHandle %s", request->url().c_str());
      return true;
    }

    return false;
  }

  void AsyncWebHandler_2::handleRequest(AsyncWebServerRequest *request)
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
