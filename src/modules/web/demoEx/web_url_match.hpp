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
#include "web_url_match.hpp"

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

        UrlMatch match_url(const std::string &url, bool only_domain = false);

    } // namespace web_server
} // namespace esphome
#endif // USE_WEBSERVER
