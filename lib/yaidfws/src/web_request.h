#pragma once

#include <esp_http_server.h>

#include <functional>
#include <map>
#include <set>
#include <string>
#include <vector>

#include <optional>

//#include "optional.h"

#include "handler.h"
#include "web_response.h"

namespace yaidfws
{

  class AsyncWebParameter
  {
  protected:
    std::string value_;

  public:
    AsyncWebParameter(std::string value) : value_(std::move(value)) {}
    const std::string &value() const { return this->value_; }
  };





  // TODO перенес код из утилит сюда. сделал отложенный UrlParser, по мере необходимости
  class AsyncWebServerRequest
    {
        friend class IDFWebServer;

    protected:
        httpd_req_t *httpd_req_;
        AsyncWebServerResponse *rsp_{};
        std::map<std::string, AsyncWebParameter *> params_;
        std::string post_query_;
        mutable std::unique_ptr<UrlParser> parser_; // Lazily initialized parser

        AsyncWebServerRequest(httpd_req_t *req)
            : httpd_req_(req) {}

        AsyncWebServerRequest(httpd_req_t *req, std::string post_query)
            : httpd_req_(req), post_query_(std::move(post_query)) {}

        void init_response_(AsyncWebServerResponse *rsp, int code, const char *content_type);

    public:
        ~AsyncWebServerRequest();

        http_method method() const { return static_cast<http_method>(this->httpd_req_->method); }

        // Access the parser lazily
        const UrlParser &getParser() const
        {
            if (!parser_)
            {
                //parser_ = std::make_unique<UrlParserRegex>(httpd_req_->uri); 
                parser_ = std::make_unique<UrlParserManual>(httpd_req_->uri); 
                ESP_LOGI("REQUEST","getParser %s", parser_->toStr().c_str());
            }
            return *parser_;
        }

        std::string url() const { return getParser().getPath(); };
        std::string host() const; // { return getParser().getHost(); };

        size_t contentLength() const { return httpd_req_->content_len; }
        int getMethod() const { return httpd_req_->method; }
        httpd_req_t *getHttpdReq() const { return httpd_req_; }

        bool request_has_header(httpd_req_t *req, const char *name);
        std::optional<std::string> request_get_header(httpd_req_t *req, const char *name);
        [[deprecated("Use UrlParser instead")]]
        std::optional<std::string> request_get_url_query(httpd_req_t *req);
        [[deprecated("Use UrlParser instead")]]
        std::optional<std::string> query_key_value(const std::string &query_url, const std::string &key);

        std::optional<std::string> get_header(const char *name) const;
        bool hasHeader(const char *name) const;

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
    };
} // namespace yaidfws_idf
