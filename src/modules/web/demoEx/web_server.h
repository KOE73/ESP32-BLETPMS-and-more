#pragma once

#include "list_entities.h"

#include "web_server_base.h"
#ifdef USE_WEBSERVER
// #include "esphome/core/component.h"
// #include "esphome/core/controller.h"
// #include "esphome/core/entity_base.h"

#include <map>
#include <vector>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <deque>

#if USE_WEBSERVER_VERSION >= 2
extern const uint8_t ESPHOME_WEBSERVER_INDEX_HTML[] /*PROGMEM*/;
extern const size_t ESPHOME_WEBSERVER_INDEX_HTML_SIZE;
#endif

#ifdef USE_WEBSERVER_CSS_INCLUDE
extern const uint8_t ESPHOME_WEBSERVER_CSS_INCLUDE[] PROGMEM;
extern const size_t ESPHOME_WEBSERVER_CSS_INCLUDE_SIZE;
#endif

#ifdef USE_WEBSERVER_JS_INCLUDE
extern const uint8_t ESPHOME_WEBSERVER_JS_INCLUDE[] PROGMEM;
extern const size_t ESPHOME_WEBSERVER_JS_INCLUDE_SIZE;
#endif

namespace esphome
{
  namespace web_server
  {

    /// Internal helper struct that is used to parse incoming URLs
    /// Тут находится результат работы разбора url
    struct UrlMatch
    {
      std::string domain; ///< The domain of the component, for example "sensor"
      std::string id;     ///< The id of the device that's being accessed, for example "living_room_fan"
      std::string method; ///< The method that's being called, for example "turn_on"
      bool valid;         ///< Whether this match is valid
    };

    struct SortingComponents
    {
      float weight;
      uint64_t group_id;
    };

    struct SortingGroup
    {
      std::string name;
      float weight;
    };

    enum JsonDetail
    {
      DETAIL_ALL,
      DETAIL_STATE
    };

    class AsyncWebHandler_WebServer : public AsyncWebHandler // public Controller, public Component,
    {
            /// Override the web handler's canHandle method.
      bool canHandle(AsyncWebServerRequest *request) override;
      /// Override the web handler's handleRequest method.
      void handleRequest(AsyncWebServerRequest *request) override;
      /// This web handle is not trivial.
      bool isRequestHandlerTrivial() override; 



           /// Handle an index request under '/'.
      void handle_index_request(AsyncWebServerRequest *request);

    };

    /** This class allows users to create a web server with their ESP nodes.
     *
     * Behind the scenes it's using AsyncWebServer to set up the server. It exposes 3 things:
     * an index page under '/' that's used to show a simple web interface (the css/js is hosted
     * by esphome.io by default), an event source under '/events' that automatically sends
     * all state updates in real time + the debug log. Lastly, there's an REST API available
     * under the '/light/...', '/sensor/...', ... URLs. A full documentation for this API
     * can be found under https://esphome.io/web-api/index.html.
     */
    class WebServerControllerComponent //: public AsyncWebHandler // public Controller, public Component,
    {

    protected:
      void schedule_(std::function<void()> &&f);
      friend ListEntitiesIterator;
      web_server_base::WebServerBaseComponent *_webServerBaseComponent;

      AsyncWebHandlerEventSource events_{"/events"};

// Пока временно тут
      AsyncWebHandler_WebServer _main_handler;

      ListEntitiesIterator entities_iterator_;
      // std::map<EntityBase *, SortingComponents> sorting_entitys_;
      // std::map<uint64_t, SortingGroup> sorting_groups_;

#if USE_WEBSERVER_VERSION == 1
      const char *css_url_{nullptr};
      const char *js_url_{nullptr};
#endif
#ifdef USE_WEBSERVER_CSS_INCLUDE
      const char *css_include_{nullptr};
#endif
#ifdef USE_WEBSERVER_JS_INCLUDE
      const char *js_include_{nullptr};
#endif
      bool allow_ota_{true};
      bool expose_log_{true};
#ifdef USE_ESP32
      std::deque<std::function<void()>> to_schedule_;
      SemaphoreHandle_t to_schedule_lock_;
#endif

    public:
      WebServerControllerComponent(web_server_base::WebServerBaseComponent *base);



      // ========== INTERNAL METHODS ==========
      // (In most use cases you won't need these)
      /// Setup the internal web server and register handlers.
      // Пинает основного владельца сервера и тот запускает сервер
      // Присоединяет свои базовые обработчики, которые отвечают за страницу и ота и прочие обязатеьльности
      // Также обработчик от AsyncWebHandler_WebServer, который раньше входил с состав этого класса
      // Будет переосмысливаться
      void setup() ;/*override*/


      void loop() ;/*override*/

      void dump_config() /*override*/;


#if USE_WEBSERVER_VERSION == 1
      /** Set the URL to the CSS <link> that's sent to each client. Defaults to
       * https://esphome.io/_static/webserver-v1.min.css
       *
       * @param css_url The url to the web server stylesheet.
       */
      void set_css_url(const char *css_url);

      /** Set the URL to the script that's embedded in the index page. Defaults to
       * https://esphome.io/_static/webserver-v1.min.js
       *
       * @param js_url The url to the web server script.
       */
      void set_js_url(const char *js_url);
#endif

#ifdef USE_WEBSERVER_CSS_INCLUDE
      /** Set local path to the script that's embedded in the index page. Defaults to
       *
       * @param css_include Local path to web server script.
       */
      void set_css_include(const char *css_include);
#endif

#ifdef USE_WEBSERVER_JS_INCLUDE
      /** Set local path to the script that's embedded in the index page. Defaults to
       *
       * @param js_include Local path to web server script.
       */
      void set_js_include(const char *js_include);
#endif



      /** Set whether or not the webserver should expose the OTA form and handler.
       *
       * @param allow_ota.
       */
      void set_allow_ota(bool allow_ota) { this->allow_ota_ = allow_ota; }

      /** Set whether or not the webserver should expose the Log.
       *
       * @param expose_log.
       */
      void set_expose_log(bool expose_log) { this->expose_log_ = expose_log; }


      /// MQTT setup priority.
      float get_setup_priority() const /*override*/;

 
      /// Return the webserver configuration as JSON.
      std::string get_config_json();

#ifdef USE_WEBSERVER_CSS_INCLUDE
      /// Handle included css request under '/0.css'.
      void handle_css_request(AsyncWebServerRequest *request);
#endif

#ifdef USE_WEBSERVER_JS_INCLUDE
      /// Handle included js request under '/0.js'.
      void handle_js_request(AsyncWebServerRequest *request);
#endif

#ifdef USE_WEBSERVER_PRIVATE_NETWORK_ACCESS
      // Handle Private Network Access CORS OPTIONS request
      void handle_pna_cors_request(AsyncWebServerRequest *request);
#endif

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



      // void add_entity_config(EntityBase *entity, float weight, uint64_t group);
      // void add_sorting_group(uint64_t group_id, const std::string &group_name, float weight);
    };

  } // namespace web_server
} // namespace esphome
#endif
