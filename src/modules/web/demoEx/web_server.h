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

 

       
        };

    } // namespace web_server
} // namespace esphome
#endif
