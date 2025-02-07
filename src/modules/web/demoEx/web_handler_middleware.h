#pragma once

#define USE_NETWORK

// #include "esphome/core/defines.h"
#ifdef USE_NETWORK
#include <memory>
#include <utility>
#include <vector>

// #include "esphome/core/component.h"

// #include "esphome/core/hal.h"
#include "web_server_idf.h"

#include "web_handler.h"

namespace esphome
{
  namespace web_server_base
  {

    class MiddlewareHandler : public AsyncWebHandler
    {
    public:
      MiddlewareHandler(AsyncWebHandler *next) : next_(next) {}

      bool canHandle(AsyncWebServerRequest *request) override { return next_->canHandle(request); }
      void handleRequest(AsyncWebServerRequest *request) override { next_->handleRequest(request); }
      void handleUpload(AsyncWebServerRequest *request, const String &filename, size_t index, uint8_t *data, size_t len,
                        bool final) override
      {
        next_->handleUpload(request, filename, index, data, len, final);
      }
      void handleBody(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) override
      {
        next_->handleBody(request, data, len, index, total);
      }
      bool isRequestHandlerTrivial() override { return next_->isRequestHandlerTrivial(); }

    protected:
      AsyncWebHandler *next_;
    };

    struct Credentials
    {
      std::string username;
      std::string password;
    };

    class AuthMiddlewareHandler : public MiddlewareHandler
    {
    protected:
      Credentials *credentials_;

    public:
      AuthMiddlewareHandler(AsyncWebHandler *next, Credentials *credentials)
          : MiddlewareHandler(next), credentials_(credentials) {}

      bool check_auth(AsyncWebServerRequest *request)
      {
        bool success = request->authenticate(credentials_->username.c_str(), credentials_->password.c_str());
        if (!success)
        {
          request->requestAuthentication();
        }
        return success;
      }

      void handleRequest(AsyncWebServerRequest *request) override
      {
        if (!check_auth(request))
          return;
        MiddlewareHandler::handleRequest(request);
      }
      void handleUpload(AsyncWebServerRequest *request, const String &filename, size_t index, uint8_t *data, size_t len,
                        bool final) override
      {
        if (!check_auth(request))
          return;
        MiddlewareHandler::handleUpload(request, filename, index, data, len, final);
      }
      void handleBody(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) override
      {
        if (!check_auth(request))
          return;
        MiddlewareHandler::handleBody(request, data, len, index, total);
      }
    };


  } // namespace web_server_base
} // namespace esphome
#endif
