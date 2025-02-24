#pragma once

//#include <string>
#include "esp_event.h"

// #define SYS_INNER_EVENT_BASE "SYS_INNER_EVENT"

ESP_EVENT_DECLARE_BASE(SYS_INNER_EVENT);
// static const char* MY_EVENT_BASE = "MY_EVENT_BASE";
typedef enum //sys_inner_events
{
    SYS_INNER_EVENT_WS_SEND_JSON = 0
} sys_inner_events_t;

/**
 * @brief Argument structure for SYS_INNER_EVENT_WS_SEND_JSON event
 */
typedef struct
{
    //std::string json;
} sys_inner_event_ws_send_json_t;
