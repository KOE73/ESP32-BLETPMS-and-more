#pragma once

// #include "list_entities.h"

#ifdef USE_WEBSERVER
// #include "esphome/core/component.h"
// #include "esphome/core/controller.h"
// #include "esphome/core/entity_base.h"

#include <map>
#include <vector>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <deque>

#include "web_handler_main.h"
#include "web_events.h"

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

        enum JsonDetail
        {
            DETAIL_ALL,
            DETAIL_STATE
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

            web_server_base::WebServerBaseComponent *_webServerBaseComponent;

            AsyncWebHandlerEventSource events_{"/events"};

            // Пока временно тут
            AsyncWebHandler_WebServer _main_handler;


#ifdef USE_WEBSERVER_CSS_INCLUDE
            const char *css_include_{nullptr};
#endif
#ifdef USE_WEBSERVER_JS_INCLUDE
            const char *js_include_{nullptr};
#endif

            std::deque<std::function<void()>> to_schedule_;
            SemaphoreHandle_t to_schedule_lock_;

            bool allow_ota_{true};
            bool expose_log_{true};

        public:
            WebServerControllerComponent(web_server_base::WebServerBaseComponent *base);

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

#ifdef USE_WEBSERVER_CSS_INCLUDE
            void set_css_include(const char *css_include) { this->css_include_ = css_include; }
#endif
#ifdef USE_WEBSERVER_JS_INCLUDE
            void set_js_include(const char *js_include) { this->js_include_ = js_include; }
#endif

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

        };

    } // namespace web_server
} // namespace esphome
#endif
