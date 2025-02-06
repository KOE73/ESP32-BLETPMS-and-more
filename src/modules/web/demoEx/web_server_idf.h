#pragma once

#include <esp_http_server.h>

#include <functional>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "optional.h"

namespace esphome
{
  namespace web_server_idf
  {

#define F(string_literal) (string_literal)
#define PGM_P const char *
#define strncpy_P strncpy

    using String = std::string;

    class AsyncWebParameter
    {
    public:
      AsyncWebParameter(std::string value) : value_(std::move(value)) {}
      const std::string &value() const { return this->value_; }

    protected:
      std::string value_;
    };

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

    using AsyncEventSourceClient = AsyncEventSourceResponse;

    class AsyncWebHandlerEventSource : public AsyncWebHandler
    {
      friend class AsyncEventSourceResponse;
      using connect_handler_t = std::function<void(AsyncEventSourceClient *)>;

    public:
      AsyncWebHandlerEventSource(std::string url) : url_(std::move(url)) {}
      ~AsyncWebHandlerEventSource() override;

      bool canHandle(AsyncWebServerRequest *request) override
      {
        return request->method() == HTTP_GET && request->url() == this->url_;
      }

      void handleRequest(AsyncWebServerRequest *request) override;

      void onConnect(connect_handler_t cb) { this->on_connect_ = std::move(cb); }

      void send(const char *message, const char *event = nullptr, uint32_t id = 0, uint32_t reconnect = 0);

      size_t count() const { return this->sessions_.size(); }

    protected:
      std::string url_;
      std::set<AsyncEventSourceResponse *> sessions_;
      connect_handler_t on_connect_{};
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

using namespace esphome::web_server_idf; // NOLINT(google-global-names-in-headers)
