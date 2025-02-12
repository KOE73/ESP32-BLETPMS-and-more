#pragma once

#include <esp_http_server.h>

#include <functional>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "optional.h"

#include "web_handler.h"

namespace esphome
{
  namespace web_server
  {


    class AsyncWebHandlerEventSource;

    class AsyncEventSourceResponse
    {
      friend class AsyncWebHandlerEventSource;

    public:
      void send(const char *message, const char *event = nullptr, uint32_t id = 0, uint32_t reconnect = 0);

    protected:
      AsyncEventSourceResponse(const AsyncWebServerRequest *request, AsyncWebHandlerEventSource *server);
      static void destroy(void *p);
      AsyncWebHandlerEventSource *_eventSource;
      httpd_handle_t _httpd_handle{};
      int fd_{};
    };


    class AsyncWebHandlerEventSource : public AsyncWebHandler
    {
      friend class AsyncEventSourceResponse;
      using connect_handler_t = std::function<void(AsyncEventSourceResponse *)>;

    public:
      AsyncWebHandlerEventSource(std::string url) : url_(std::move(url)) {}
      ~AsyncWebHandlerEventSource() override;

      bool canHandle(AsyncWebServerRequest *request) override
      {
        return request->method() == HTTP_GET && request->url() == this->url_;
      }

      void handleRequest(AsyncWebServerRequest *request) override;

      void onConnect(connect_handler_t cb) { this->on_connect_ = std::move(cb); }

      void send(const char *message, const char *event = nullptr, uint32_t id = 0, uint32_t reconnect = 0) const;

      size_t count() const { return this->sessions_.size(); }

    protected:
      std::string url_;
      std::set<AsyncEventSourceResponse *> sessions_;
      connect_handler_t on_connect_{};
    };

  } // namespace web_server_idf
} // namespace esphome

using namespace esphome::web_server; // NOLINT(google-global-names-in-headers)
