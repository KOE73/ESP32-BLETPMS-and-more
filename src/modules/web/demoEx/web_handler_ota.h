#pragma once

// #include "esphome/core/defines.h"
#include <memory>
#include <utility>
#include <vector>

// #include "esphome/core/component.h"

// #include "esphome/core/hal.h"
#include "web_server_idf.h"
#include "web_handler.h"

namespace web_server
{

  class OTARequestHandler : public AsyncWebHandler
  {
  public:
    OTARequestHandler(WebServerContainer *parent) : parent_(parent) {}
    void handleRequest(AsyncWebServerRequest *request) override;
    void handleUpload(AsyncWebServerRequest *request, const String &filename, size_t index, uint8_t *data, size_t len,
                      bool final) override;
    bool canHandle(AsyncWebServerRequest *request) override
    {
      return request->url() == "/update" && request->method() == HTTP_POST;
    }

    // XxLL
    bool isRequestHandlerTrivial() override { return false; }

  protected:
    uint32_t last_ota_progress_{0};
    uint32_t ota_read_length_{0};
    WebServerContainer *parent_;
  };

} // namespace web_server
