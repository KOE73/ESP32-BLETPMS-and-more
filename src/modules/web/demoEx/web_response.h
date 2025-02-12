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


    class AsyncWebServerRequest;

    class AsyncWebServerResponse
    {
    protected:
      const AsyncWebServerRequest *req_;

    public:
      AsyncWebServerResponse(const AsyncWebServerRequest *req) : req_(req) {}
      virtual ~AsyncWebServerResponse() {}

      void addHeader(const char *name, const char *value);

      virtual const char *get_content_data() const = 0;
      virtual size_t get_content_size() const = 0;
    };

    class AsyncWebServerResponseEmpty : public AsyncWebServerResponse
    {
    public:
      AsyncWebServerResponseEmpty(const AsyncWebServerRequest *req) : AsyncWebServerResponse(req) {}

      const char *get_content_data() const override { return nullptr; };
      size_t get_content_size() const override { return 0; };
    };

    class AsyncWebServerResponseContent : public AsyncWebServerResponse
    {
    protected:
      std::string content_;

    public:
      AsyncWebServerResponseContent(const AsyncWebServerRequest *req, std::string content)
          : AsyncWebServerResponse(req), content_(std::move(content)) {}

      const char *get_content_data() const override { return this->content_.c_str(); };
      size_t get_content_size() const override { return this->content_.size(); };
    };

    class AsyncResponseStream : public AsyncWebServerResponse
    {
    protected:
      std::string content_;

    public:
      AsyncResponseStream(const AsyncWebServerRequest *req) : AsyncWebServerResponse(req) {}

      const char *get_content_data() const override { return this->content_.c_str(); };
      size_t get_content_size() const override { return this->content_.size(); };

      void print(const char *str) { this->content_.append(str); }
      void print(const std::string &str) { this->content_.append(str); }
      void print(float value);
      void printf(const char *fmt, ...) __attribute__((format(printf, 2, 3)));
    };

    class AsyncWebServerResponseProgmem : public AsyncWebServerResponse
    {
    protected:
      const uint8_t *data_;
      size_t size_;

    public:
      AsyncWebServerResponseProgmem(const AsyncWebServerRequest *req, const uint8_t *data, const size_t size)
          : AsyncWebServerResponse(req), data_(data), size_(size) {}

      const char *get_content_data() const override { return reinterpret_cast<const char *>(this->data_); };
      size_t get_content_size() const override { return this->size_; };
    };

  } // namespace web_server_idf
} // namespace esphome

using namespace esphome::web_server; // NOLINT(google-global-names-in-headers)
