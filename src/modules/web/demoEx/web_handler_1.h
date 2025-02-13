#pragma once

#ifdef USE_WEBSERVER

#include <map>
#include <vector>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <deque>

// #include "web_server_container.h"
#include "web_handler.h"


namespace esphome
{
  namespace web_server
  {

    /// @brief For static text
    class AsyncWebHandler_1 : public AsyncWebHandler // public Controller, public Component,
    {
    private:
      const char *_uri;
      const char *_text;
      ssize_t _text_len;

    protected:
      /// Override the web handler's canHandle method.
      bool canHandle(AsyncWebServerRequest *request) override;
      /// Override the web handler's handleRequest method.
      void handleRequest(AsyncWebServerRequest *request) override;
      /// This web handle is not trivial.
      bool isRequestHandlerTrivial() override;

    public:
      AsyncWebHandler_1(const char *uri);
      AsyncWebHandler_1(const char *uri, const char *text);
      AsyncWebHandler_1(const char *uri, const char *text, ssize_t buf_len);
    };


    /// @brief For binary and may be gziped
    class AsyncWebHandler_2 : public AsyncWebHandler // public Controller, public Component,
    {
    private:
      const char *_uri;
      const char *_text;
      ssize_t _text_len;

    protected:
      /// Override the web handler's canHandle method.
      bool canHandle(AsyncWebServerRequest *request) override;
      /// Override the web handler's handleRequest method.
      void handleRequest(AsyncWebServerRequest *request) override;
      /// This web handle is not trivial.
      bool isRequestHandlerTrivial() override;

    public:
      AsyncWebHandler_2(const char *uri);
      AsyncWebHandler_2(const char *uri, const uint8_t *text);
      AsyncWebHandler_2(const char *uri, const uint8_t *text, ssize_t buf_len);
    };

  } // namespace web_server
} // namespace esphome
#endif // USE_WEBSERVER
