#pragma once

// #include "esphome/core/defines.h"

#include <memory>
#include <utility>
#include <vector>
#include <algorithm>

#ifdef USE_LOGGER
#include "esphome/components/logger/logger.h"
#endif

// #include "esphome/core/component.h"

// #include "esphome/core/hal.h"

#include "web_server_idf.h"
// #include "web_handler.h"
#include "web_handler_main.h"
#include "web_events.h"
#include "web_ws.h"
#include "web_handler_middleware.h"

namespace web_server
{
  // ??? Разобраться с h
  class AsyncWebHandler_WebServer;

  /// @brief Либо не нужен либо творчески переработать
  /// Именн этот класс владел реальным веб сервером с портом и прочим
  ///  а ткаже настройками пользователя и пароля
  /// ОБработчики добавляемые через него midleware используют имя пароль
  class WebServerContainer
  {
  protected:
    friend class OTARequestHandler;

    int initialized_{0};
    const uint16_t port_{80};
    std::shared_ptr<IDFWebServer> _IDFWebServer{nullptr};
    std::vector<AsyncWebHandler *> handlers_;
    Credentials credentials_;

#pragma region To refactor
  protected:
    void schedule_(std::function<void()> &&f);

    // web_server::WebServerContainer *_webServerBaseComponent;

    // Why inside? Move it out as a regular handler.
    // If you want it inside, you can make a virtual initializer
    AsyncWebHandlerEventSource events_{"/events"};
    AsyncWebHandlerWSSource ws_{"/ws"};

    // Пока временно тут
    AsyncWebHandler_WebServer _main_handler;

    std::deque<std::function<void()>> to_schedule_;
    SemaphoreHandle_t to_schedule_lock_;

    bool allow_ota_{true};
    bool expose_log_{true};
#pragma endregion

  public:
    WebServerContainer();
    WebServerContainer(uint16_t port);
    ~WebServerContainer();

    const AsyncWebHandlerEventSource &getEvents() const { return events_; }
    const AsyncWebHandlerWSSource &getWS() const { return ws_; }

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

    void set_auth_username(std::string auth_username) { credentials_.username = std::move(auth_username); }
    void set_auth_password(std::string auth_password) { credentials_.password = std::move(auth_password); }

    // Добавление обработчика. Если ест имя пароль, то обработчик оборачивается в midleware
    // обработчики хранятся локально
    // если есть сервер, то сразу присоединяется к серверу
    void add_handler(AsyncWebHandler *handler);

    void add_ota_handler();

#pragma region To refactor

    void set_allow_ota(bool allow_ota) { this->allow_ota_ = allow_ota; }
    void set_expose_log(bool expose_log) { this->expose_log_ = expose_log; }

    // ========== INTERNAL METHODS ==========
    // (In most use cases you won't need these)
    /// Setup the internal web server and register handlers.
    // Пинает основного владельца сервера и тот запускает сервер
    // Присоединяет свои базовые обработчики, которые отвечают за страницу и ота и прочие обязатеьльности
    // Также обработчик от AsyncWebHandler_WebServer, который раньше входил с состав этого класса
    // Будет переосмысливаться
    void setup(); /*override*/

    void loop(); /*override*/

    void dump_config() /*override*/;

    /// Return the webserver configuration as JSON.
    std::string get_config_json();

#ifdef USE_WEBSERVER_PRIVATE_NETWORK_ACCESS
    // Handle Private Network Access CORS OPTIONS request
    void handle_pna_cors_request(AsyncWebServerRequest *request);
#endif

#pragma endregion
  };

} // namespace web_server
