#pragma once

#include <map>
#include <vector>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <deque>

// #include "web_server_container.h"
#include "web_handler.h"

#if USE_WEBSERVER_VERSION >= 2
extern const uint8_t ESPHOME_WEBSERVER_INDEX_HTML[] /*PROGMEM*/;
extern const size_t ESPHOME_WEBSERVER_INDEX_HTML_SIZE;
#endif


namespace web_server
{

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

  };

} // namespace web_server
