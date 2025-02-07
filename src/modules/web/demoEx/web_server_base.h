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
#include "web_handler_middleware.h"


namespace esphome
{
  namespace web_server_base
  {

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
      Credentials credentials_;

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


  } // namespace web_server_base
} // namespace esphome
#endif
