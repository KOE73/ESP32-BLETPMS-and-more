#pragma once

#include <esp_http_server.h>

#include <functional>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "web_request.h"


namespace esphome
{
  namespace web_server_idf
  {

    class AsyncWebHandler
    {
    public:
      virtual ~AsyncWebHandler() {}
      virtual bool canHandle(AsyncWebServerRequest *request) { return false; }
      virtual void handleRequest(AsyncWebServerRequest *request) {}
      virtual void handleUpload(AsyncWebServerRequest *request, const std::string &filename, size_t index, uint8_t *data,
                                size_t len, bool final) {}
      virtual void handleBody(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {}
      virtual bool isRequestHandlerTrivial() { return true; }
    };

   
  } // namespace web_server_idf
} // namespace esphome

