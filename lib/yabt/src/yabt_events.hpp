#pragma once

#include "esp_event.h"

namespace yabt
{
    ESP_EVENT_DECLARE_BASE(YABT_EVENT);

    typedef enum yabt_events
    {
        YABT_EVENT_TPMS = 0, // TPMSData
        YABT_EVENT_TOMTOM_TPMS = YABT_EVENT_TPMS + 1,
    } yabt_events_t;

} // namespace yabt