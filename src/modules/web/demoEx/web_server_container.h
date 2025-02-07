#pragma once

// #include "esphome/core/defines.h"

#include <memory>
#include <utility>
#include <vector>
#include <algorithm>

#ifdef USE_LOGGER
#include "esphome/components/logger/logger.h"
#endif

#ifdef USE_WEBSERVER_LOCAL
#if USE_WEBSERVER_VERSION == 2
#include "server_index_v2.h"
#elif USE_WEBSERVER_VERSION == 3
#include "server_index_v3.h"
#endif
#endif

// #include "esphome/core/component.h"

// #include "esphome/core/hal.h"

#include "web_server_idf.h"
//#include "web_handler.h"
#include "web_handler_main.h"
#include "web_events.h"
#include "web_handler_middleware.h"

namespace esphome
{
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
      uint16_t port_{80};
      std::shared_ptr<IDFWebServer> _IDFWebServer{nullptr};
      std::vector<AsyncWebHandler *> handlers_;
      Credentials credentials_;

#pragma region To refactor
    protected:
      void schedule_(std::function<void()> &&f);

      //web_server::WebServerContainer *_webServerBaseComponent;

      AsyncWebHandlerEventSource events_{"/events"};

      // Пока временно тут
      AsyncWebHandler_WebServer _main_handler;

      std::deque<std::function<void()>> to_schedule_;
      SemaphoreHandle_t to_schedule_lock_;

      bool allow_ota_{true};
      bool expose_log_{true};
#pragma endregion

    public:
      WebServerContainer();

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

      void set_port(uint16_t port) { port_ = port; }
      uint16_t get_port() const { return port_; }

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

#ifdef USE_WEBSERVER_CSS_INCLUDE
    protected:
      const char *css_include_{nullptr};

    public:
      void set_css_include(const char *css_include) { this->css_include_ = css_include; }
      void set_css_include(const char *css_include);
      /// Handle included css request under '/0.css'.
      void handle_css_request(AsyncWebServerRequest *request);
#endif
#ifdef USE_WEBSERVER_JS_INCLUDE
    protected:
      const char *js_include_{nullptr};

    public:
      void set_js_include(const char *js_include) { this->js_include_ = js_include; }
      void set_js_include(const char *js_include);
      /// Handle included js request under '/0.js'.
      void handle_js_request(AsyncWebServerRequest *request);
#endif

#ifdef USE_WEBSERVER_PRIVATE_NETWORK_ACCESS
      // Handle Private Network Access CORS OPTIONS request
      void handle_pna_cors_request(AsyncWebServerRequest *request);
#endif

#pragma region Obsolete Example

#ifdef USE_SENSOR
      void on_sensor_update(sensor::Sensor *obj, float state) /*override*/;
      /// Handle a sensor request under '/sensor/<id>'.
      void handle_sensor_request(AsyncWebServerRequest *request, const UrlMatch &match);

      /// Dump the sensor state with its value as a JSON string.
      std::string sensor_json(sensor::Sensor *obj, float value, JsonDetail start_config);
#endif

#ifdef USE_SWITCH
      void on_switch_update(switch_::Switch *obj, bool state) /*override*/;

      /// Handle a switch request under '/switch/<id>/</turn_on/turn_off/toggle>'.
      void handle_switch_request(AsyncWebServerRequest *request, const UrlMatch &match);

      /// Dump the switch state with its value as a JSON string.
      std::string switch_json(switch_::Switch *obj, bool value, JsonDetail start_config);
#endif

#ifdef USE_BUTTON
      /// Handle a button request under '/button/<id>/press'.
      void handle_button_request(AsyncWebServerRequest *request, const UrlMatch &match);

      /// Dump the button details with its value as a JSON string.
      std::string button_json(button::Button *obj, JsonDetail start_config);
#endif

#ifdef USE_EVENT
      void on_event(event::Event *obj, const std::string &event_type) /*override*/;

      /// Handle a event request under '/event<id>'.
      void handle_event_request(AsyncWebServerRequest *request, const UrlMatch &match);

      /// Dump the event details with its value as a JSON string.
      std::string event_json(event::Event *obj, const std::string &event_type, JsonDetail start_config);
#endif

#ifdef USE_UPDATE
      void on_update(update::UpdateEntity *obj) /*override*/;

      /// Handle a update request under '/update/<id>'.
      void handle_update_request(AsyncWebServerRequest *request, const UrlMatch &match);

      /// Dump the update state with its value as a JSON string.
      std::string update_json(update::UpdateEntity *obj, JsonDetail start_config);
#endif

#pragma endregion
#pragma endregion
    };

  } // namespace web_server
} // namespace esphome
