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
    /// @brief Abstract event handler template for specific device data types.
    ///
    /// This class serves as a generic base for event handling. It tracks incoming
    /// device data, compares it with the previously stored state, and triggers an
    /// update when changes occur. This is particularly useful for UI updates where
    /// only changed data should be reflected in the interface.
    template <typename T>
    class EventHandler
    {
    protected:
        T lastEvent;                ///< Stores the last processed event data.
        bool isInitialized = false; ///< Tracks whether the first event has been processed.

    public:
        virtual ~EventHandler() = default;

        /// @brief Processes incoming event data and checks for changes.
        /// @param data The new data event to be handled.
        virtual void handleEvent(const T &data)
        {
            if (!isInitialized || memcmp(&lastEvent, &data, sizeof(T)) != 0)
            {
                lastEvent = data;
                isInitialized = true;
                onDataChanged(data);
            }
        }

    protected:
        /// @brief Called when new data differs from the last stored event.
        /// @param data The new data triggering the update.
        virtual void onDataChanged(const T &data) = 0;
    };

    /// @brief Data structure that holds style-related information for LVGL updates.
    struct GenericData : public yabt::DeviceData
    {
        LVGLStyle style; ///< Style settings for LVGL elements.
        GenericData():DeviceData("GenericData"){}
    };

    /// @brief LVGL Event Handler managing UI updates for TPMS data and styles.
    ///
    /// This class handles TPMS sensor data and applies style settings dynamically
    /// based on received event updates. It ensures that UI updates occur only
    /// when there are actual changes in the data.
    class LVGLHandler : public EventHandler<yabt::TPMSData>, public EventHandler<GenericData>
    {
    private:
        lv_obj_t *container;        ///< UI container where labels are displayed.
        lv_obj_t *ui_Panel;         ///< UI container where labels are displayed.
        lv_obj_t *pressureLabel;    ///< Label for displaying pressure.
        lv_obj_t *temperatureLabel; ///< Label for displaying temperature.
        lv_style_t style;

    public:
        /// @brief Constructor initializes the handler with an external container.
        /// @param parentContainer The parent LVGL container where elements will be placed.
        LVGLHandler(lv_obj_t *parentContainer);

        static void eventHandler(void *handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data);

        static void genericEventHandler(void *handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data);

        void handleTPMSEvent(const yabt::TPMSData &data);
        void handleGenericEvent(const GenericData &data);

    protected:
        void onDataChanged(const yabt::TPMSData &data) override;
        void onDataChanged(const GenericData &data) override;

    private:
        void createLabels();
        void applyStyle(lv_obj_t *obj, const LVGLStyle &style);
    };
} // namespace lcd
