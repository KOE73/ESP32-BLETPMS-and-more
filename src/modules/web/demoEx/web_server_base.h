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

namespace esphome
{
  namespace web_server_base
  {

    namespace internal
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

    } // namespace internal

    /// @brief Либо не нужен либо творчески переработать
    /// Именн этот класс владел реальным веб сервером с портом и прочим
    ///  а ткаже настройками пользователя и пароля
    /// ОБработчики добавляемые через него midleware используют имя пароль
    class WebServerBaseComponent
    {
    protected:
      friend class OTARequestHandler;

      int initialized_{0};
      uint16_t port_{80};
      std::shared_ptr<IDFWebServer> _IDFWebServer{nullptr};
      std::vector<AsyncWebHandler *> handlers_;
      internal::Credentials credentials_;

    public:
      // Запуск idf web сервера и присоединение локальных обработчиков
      void init()
      {
        if (this->initialized_)
        {
          this->initialized_++;
          return;
        }

        // Делаем сервер
        this->_IDFWebServer = std::make_shared<IDFWebServer>(this->port_);

        // All content is controlled and created by user - so allowing all origins is fine here.
        DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");

        // Запускаем сервер
        this->_IDFWebServer->begin();

        for (auto *handler : this->handlers_)
          this->_IDFWebServer->addHandler(handler);

        this->initialized_++;
      }

      void deinit()
      {
        this->initialized_--;
        if (this->initialized_ == 0)
        {
          this->_IDFWebServer = nullptr;
        }
      }
      std::shared_ptr<IDFWebServer> get_server() const { return _IDFWebServer; }
      float get_setup_priority() const;

      void set_auth_username(std::string auth_username) { credentials_.username = std::move(auth_username); }
      void set_auth_password(std::string auth_password) { credentials_.password = std::move(auth_password); }

      // Добавление обработчика. Если ест имя пароль, то обработчик оборачивается в midleware
      // обработчики хранятся локально
      // если есть сервер, то сразу присоединяется к серверу
      void add_handler(AsyncWebHandler *handler);

      void add_ota_handler();

      void set_port(uint16_t port) { port_ = port; }
      uint16_t get_port() const { return port_; }
    };

    class OTARequestHandler : public AsyncWebHandler
    {
    public:
      OTARequestHandler(WebServerBaseComponent *parent) : parent_(parent) {}
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
      WebServerBaseComponent *parent_;
    };

  } // namespace web_server_base
} // namespace esphome
#endif
