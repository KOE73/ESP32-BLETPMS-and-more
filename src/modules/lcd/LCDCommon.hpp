// LCD Event Handling - Header File
// Provides event-driven data updates for LVGL UI components

#pragma once

#include <iostream>
#include <cstring>

#include "esp_event.h"

#include "lvgl.h"

#include "yabt.hpp"
#include "yabt_tpms.hpp"

namespace lcd
{
    
    /// @brief Structure representing LVGL-compatible style settings.
    struct LVGLStyle
    {
        lv_color_t textColor; ///< Text color of the label.
        lv_color_t bgColor;   ///< Background color of the container.
        lv_align_t alignment; ///< Alignment of the UI elements.
    };

   
} // namespace lcd
