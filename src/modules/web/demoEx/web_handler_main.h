#pragma once

#include "web_server_base.h"

#include "web_handler.h"

#ifdef USE_WEBSERVER

#include <map>
#include <vector>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <deque>

#if USE_WEBSERVER_VERSION >= 2
extern const uint8_t ESPHOME_WEBSERVER_INDEX_HTML[] /*PROGMEM*/;
extern const size_t ESPHOME_WEBSERVER_INDEX_HTML_SIZE;
#endif

#ifdef USE_WEBSERVER_CSS_INCLUDE
extern const uint8_t ESPHOME_WEBSERVER_CSS_INCLUDE[] PROGMEM;
extern const size_t ESPHOME_WEBSERVER_CSS_INCLUDE_SIZE;
#endif

#ifdef USE_WEBSERVER_JS_INCLUDE
extern const uint8_t ESPHOME_WEBSERVER_JS_INCLUDE[] PROGMEM;
extern const size_t ESPHOME_WEBSERVER_JS_INCLUDE_SIZE;
#endif

namespace esphome
{
  namespace web_server
  {
   
    class AsyncWebHandler_WebServer : public AsyncWebHandler // public Controller, public Component,
    {
            /// Override the web handler's canHandle method.
      bool canHandle(AsyncWebServerRequest *request) override;
      /// Override the web handler's handleRequest method.
      void handleRequest(AsyncWebServerRequest *request) override;
      /// This web handle is not trivial.
      bool isRequestHandlerTrivial() override; 



           /// Handle an index request under '/'.
      void handle_index_request(AsyncWebServerRequest *request);

    };

  } // namespace web_server
} // namespace esphome
#endif // USE_WEBSERVER
