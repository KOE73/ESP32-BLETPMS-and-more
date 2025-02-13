#pragma once

#include <esp_http_server.h>

#include <functional>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "web_request.h"

namespace web_server
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

    /// @brief  Custom handlers can override this method to define whether they need to wait for the request body.
    /// @return
    /// true - for simple GET requests that do not require body processing.
    ///
    /// false - for requests that contain a body (e.g., POST, PUT, file uploads). 
    ///
    virtual bool isRequestHandlerTrivial() { return true; }
  };

} // namespace web_server_idf
