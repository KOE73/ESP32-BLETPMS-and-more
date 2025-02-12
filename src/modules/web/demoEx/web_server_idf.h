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

#define F(string_literal) (string_literal)
#define PGM_P const char *
#define strncpy_P strncpy

    using String = std::string;



    class AsyncWebServerRequest;

    class AsyncWebHandler;

    class IDFWebServer
    {
    protected:
      uint16_t port_{};
      httpd_handle_t _httpd_handle{};

      esp_err_t request_handler_(AsyncWebServerRequest *request) const;
      std::vector<AsyncWebHandler *> handlers_;
      std::function<void(AsyncWebServerRequest *request)> on_not_found_{};

#pragma region Handlers from C world to convert to C++ world
      static esp_err_t request_get_handler(httpd_req_t *r);
      static esp_err_t request_post_handler(httpd_req_t *r);
#pragma endregion

    public:
      IDFWebServer(uint16_t port) : port_(port) {};
      ~IDFWebServer() { this->end(); }

      void onNotFound(std::function<void(AsyncWebServerRequest *request)> fn) { on_not_found_ = std::move(fn); }

      void begin();
      void end();

      AsyncWebHandler &addHandler(AsyncWebHandler *handler)
      {
        this->handlers_.push_back(handler);
        return *handler;
      }
    };

    class DefaultHeaders
    {
      friend class AsyncWebServerRequest;
      friend class AsyncEventSourceResponse;

    public:
      void addHeader(const char *name, const char *value) { this->headers_.emplace_back(name, value); }

      static DefaultHeaders &Instance()
      {
        static DefaultHeaders instance;
        return instance;
      }

    protected:
      std::vector<std::pair<std::string, std::string>> headers_;
    };

  } // namespace web_server_idf
} // namespace esphome

using namespace esphome::web_server; // NOLINT(google-global-names-in-headers)
