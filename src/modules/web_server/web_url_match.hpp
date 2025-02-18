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

#endif // USE_WEBSERVER
