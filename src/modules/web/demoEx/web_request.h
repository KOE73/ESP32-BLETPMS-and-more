#pragma once

#include <esp_http_server.h>

#include <functional>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "optional.h"

#include "web_handler.h"
#include "web_response.h"

namespace esphome
{
  namespace web_server
  {

    class AsyncWebParameter
    {
    protected:
      std::string value_;

    public:
      AsyncWebParameter(std::string value) : value_(std::move(value)) {}
      const std::string &value() const { return this->value_; }
    };

    class AsyncWebServerRequest
    {
      friend class IDFWebServer;

    protected:
      httpd_req_t *req_;
      AsyncWebServerResponse *rsp_{};
      std::map<std::string, AsyncWebParameter *> params_;
      std::string post_query_;

      AsyncWebServerRequest(httpd_req_t *req) : req_(req) {}
      AsyncWebServerRequest(httpd_req_t *req, std::string post_query) : req_(req), post_query_(std::move(post_query)) {}

      void init_response_(AsyncWebServerResponse *rsp, int code, const char *content_type);

    public:
      ~AsyncWebServerRequest();

      http_method method() const { return static_cast<http_method>(this->req_->method); }
      std::string url() const;
      std::string host() const;

      size_t contentLength() const { return this->req_->content_len; }

      bool authenticate(const char *username, const char *password) const;

      void requestAuthentication(const char *realm = nullptr) const;

      void redirect(const std::string &url);

      void send(AsyncWebServerResponse *response);
      void send(int code, const char *content_type = nullptr, const char *content = nullptr);

      AsyncWebServerResponse *beginResponse(int code, const char *content_type)
      {
        auto *res = new AsyncWebServerResponseEmpty(this); // NOLINT(cppcoreguidelines-owning-memory)
        this->init_response_(res, code, content_type);
        return res;
      }

      AsyncWebServerResponse *beginResponse(int code, const char *content_type, const std::string &content)
      {
        auto *res = new AsyncWebServerResponseContent(this, content); // NOLINT(cppcoreguidelines-owning-memory)
        this->init_response_(res, code, content_type);
        return res;
      }

      AsyncWebServerResponse *beginResponse_P(int code, const char *content_type, const uint8_t *data,
                                              const size_t data_size)
      {
        auto *res = new AsyncWebServerResponseProgmem(this, data, data_size); // NOLINT(cppcoreguidelines-owning-memory)
        this->init_response_(res, code, content_type);
        return res;
      }

      AsyncResponseStream *beginResponseStream(const char *content_type)
      {
        auto *res = new AsyncResponseStream(this); // NOLINT(cppcoreguidelines-owning-memory)
        this->init_response_(res, 200, content_type);
        return res;
      }

      bool hasParam(const std::string &name) { return this->getParam(name) != nullptr; }

      AsyncWebParameter *getParam(const std::string &name);

      bool hasArg(const char *name) { return this->hasParam(name); }
      std::string arg(const std::string &name)
      {
        auto *param = this->getParam(name);
        if (param)
        {
          return param->value();
        }
        return {};
      }

      operator httpd_req_t *() const { return this->req_; }
      optional<std::string> get_header(const char *name) const;

      bool hasHeader(const char *name) const;
    };

  } // namespace web_server_idf
} // namespace esphome

using namespace esphome::web_server; // NOLINT(google-global-names-in-headers)
