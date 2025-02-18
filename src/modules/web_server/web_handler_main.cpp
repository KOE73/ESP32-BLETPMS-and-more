

#ifdef USE_WEBSERVER

#include "web_server.h"
#include "web_handler_main.h"

#include "json_util.h"
// #include "esphome/components/network/util.h"
// #include "esphome/core/application.h"
// #include "esphome/core/entity_base.h"
#include "helpers.h"
#include "esp_log.h"
// #include "esphome/core/util.h"

#include "web_url_match.hpp"

#ifdef USE_ARDUINO
#include "StreamString.h"
#endif

#include <cstdlib>

#ifdef USE_LOGGER
#include "esphome/components/logger/logger.h"
#endif

#ifdef USE_CLIMATE
#include "esphome/components/climate/climate.h"
#endif

namespace web_server
{

  static const char *const TAG = "web_server";

#pragma region AsyncWebHandler_WebServer

  bool AsyncWebHandler_WebServer::canHandle(AsyncWebServerRequest *request)
  {

    if (request->url() == "/")
    {
      ESP_LOGI(TAG, "AsyncWebHandler_WebServer::canHandle %s", request->url().c_str());
      return true;
    }

#ifdef USE_WEBSERVER_PRIVATE_NETWORK_ACCESS
    if (request->method() == HTTP_OPTIONS && request->hasHeader(HEADER_CORS_REQ_PNA))
    {
#ifdef USE_ARDUINO
      // Header needs to be added to interesting header list for it to not be
      // nuked by the time we handle the request later.
      // Only required in Arduino framework.
      request->addInterestingHeader(HEADER_CORS_REQ_PNA);
#endif
      return true;
    }
#endif

    UrlMatch match = match_url(request->url().c_str(), true);
    if (!match.valid)
      return false;
#ifdef USE_SENSOR
    if (request->method() == HTTP_GET && match.domain == "sensor")
      return true;
#endif

#ifdef USE_SWITCH
    if ((request->method() == HTTP_POST || request->method() == HTTP_GET) && match.domain == "switch")
      return true;
#endif

#ifdef USE_BUTTON
    if ((request->method() == HTTP_POST || request->method() == HTTP_GET) && match.domain == "button")
      return true;
#endif

#ifdef USE_EVENT
    if (request->method() == HTTP_GET && match.domain == "event")
      return true;
#endif

#ifdef USE_UPDATE
    if ((request->method() == HTTP_POST || request->method() == HTTP_GET) && match.domain == "update")
      return true;
#endif

    return false;
  }

  void AsyncWebHandler_WebServer::handleRequest(AsyncWebServerRequest *request)
  {
    ESP_LOGI(TAG, "AsyncWebHandler_WebServer::handleRequest %s", request->url().c_str());

    // if (request->url() == "/")
    //{
    //   this->handle_index_request(request);
    //   return;
    // }

#ifdef USE_WEBSERVER_PRIVATE_NETWORK_ACCESS
    if (request->method() == HTTP_OPTIONS && request->hasHeader(HEADER_CORS_REQ_PNA))
    {
      this->handle_pna_cors_request(request);
      return;
    }
#endif

    UrlMatch match = match_url(request->url().c_str());
#ifdef USE_SENSOR
    if (match.domain == "sensor")
    {
      this->handle_sensor_request(request, match);
      return;
    }
#endif

#ifdef USE_SWITCH
    if (match.domain == "switch")
    {
      this->handle_switch_request(request, match);
      return;
    }
#endif

#ifdef USE_BUTTON
    if (match.domain == "button")
    {
      this->handle_button_request(request, match);
      return;
    }
#endif

#ifdef USE_UPDATE
    if (match.domain == "update")
    {
      this->handle_update_request(request, match);
      return;
    }
#endif
  }

  bool AsyncWebHandler_WebServer::isRequestHandlerTrivial()
  {
    return false;
  }

#pragma endregion

} // namespace web_server

#endif // USE_WEBSERVER
