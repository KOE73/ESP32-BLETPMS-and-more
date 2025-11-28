// LCD Event Handling - Header File
// Provides event-driven data updates for LVGL UI components

#pragma once

#include <iostream>
#include <cstring>

#include "esp_event.h"

#include "lvgl.h"
#include "..\LCDCommon.hpp"

#include "yabt.hpp"
#include "yabt_tpms.hpp"

namespace lcd
{

    /// @brief LVGL Event Handler managing UI updates for TPMS data and styles.
    ///
    /// This class handles TPMS sensor data and applies style settings dynamically
    /// based on received event updates. It ensures that UI updates occur only
    /// when there are actual changes in the data.
    class LVGLHandler_Network
    {
    private:
        lv_obj_t *container;     ///< UI container where labels are displayed.
        lv_obj_t *ui_Panel;      ///< UI container where labels are displayed.
        lv_obj_t *pressureLabel; ///< Label for displaying pressure.
        lv_style_t style;

    public:
        /// @brief Constructor initializes the handler with an external container.
        /// @param parentContainer The parent LVGL container where elements will be placed.
        LVGLHandler_Network(lv_obj_t *parentContainer);

    
        void handleCBOREvent(const uint8_t *message, size_t len);

    private:
        void createLabels();
        void applyStyle(lv_obj_t *obj, const LVGLStyle &style);
    };
} // namespace lcd
