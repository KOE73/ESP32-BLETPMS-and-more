// LCD Event Handling - Implementation File
// Implements event-driven data updates for LVGL UI components

#include "lvgl_tpms.hpp"

namespace lcd
{
    /// @brief Constructor initializes the LVGL handler and creates UI labels.
    /// @param parentContainer The parent LVGL container where elements will be placed.
    LVGLHandler::LVGLHandler(lv_obj_t *parentContainer) : container(parentContainer)
    {  
        ESP_LOGI("LCD----------", "! LVGLHandler::LVGLHandler");
      
        createLabels();
        ESP_LOGI("LCD----------", "! LVGLHandler::LVGLHandler 2");
      
        ESP_ERROR_CHECK_WITHOUT_ABORT(esp_event_handler_register_with(yabt::BTController::getInstance().getEventLoop(), YABT_EVENT, ESP_EVENT_ANY_ID, &LVGLHandler::eventHandler, this));
        ESP_LOGI("LCD----------", "! LVGLHandler::LVGLHandler 3");
        // esp_event_handler_register_with(GenericEventLoop::getInstance().getEventLoop(), GENERIC_EVENT, ESP_EVENT_ANY_ID, &LVGLHandler::genericEventHandler, this);
    }

    /// @brief Creates and configures labels inside the provided container.
    void LVGLHandler::createLabels()
    {
        ESP_LOGI("LCD----------", "! Make label");
        ui_Panel = lv_obj_create(container);
        lv_obj_set_width(ui_Panel, 50);
        lv_obj_set_height(ui_Panel, 60);

        lv_style_init(&style);

        pressureLabel = lv_label_create(ui_Panel);
        lv_label_set_text(pressureLabel, "Pressure: -- Psi");
        lv_obj_align(pressureLabel, LV_ALIGN_TOP_MID, 0, 10);
        lv_obj_set_size(pressureLabel, 50, 30);
        lv_style_set_text_font(&style, &lv_font_montserrat_24);

        temperatureLabel = lv_label_create(ui_Panel);
        lv_label_set_text(temperatureLabel, "Temperature: -- °C");
        lv_obj_align(temperatureLabel, LV_ALIGN_BOTTOM_MID, 0, -10);
        lv_obj_set_size(temperatureLabel, 50, 30);
        lv_style_set_text_font(&style, &lv_font_montserrat_14);

        ESP_LOGI("LCD----------", "! Make label end");
    }

    /// @brief Handles TPMS event reception and updates UI labels.
    void LVGLHandler::handleTPMSEvent(const yabt::TPMSData &data)
    {
        EventHandler<yabt::TPMSData>::handleEvent(data);
    }

    /// @brief Handles Generic event reception and applies style changes.
    void LVGLHandler::handleGenericEvent(const GenericData &data)
    {
        EventHandler<GenericData>::handleEvent(data);
    }

    /// @brief Updates UI labels when TPMS data changes.
    void LVGLHandler::onDataChanged(const yabt::TPMSData &data)
    {
        std::cout << "TPMS Data updated: " << data.pressure_Psi << " Psi, "
                  << data.temperatureC << " °C" << std::endl;
        ESP_LOGI("LCD----------", "! Changing label");

        lv_label_set_text_fmt(pressureLabel, "Pressure: %.2f Psi", data.pressure_Psi);
        lv_label_set_text_fmt(temperatureLabel, "Temperature: %.2f °C", data.temperatureC);

        lv_obj_invalidate(ui_Panel);
        lv_obj_invalidate(pressureLabel);

        ESP_LOGI("LCD----------", "! Chenged label");
    }

    /// @brief Applies style changes when GenericData is updated.
    void LVGLHandler::onDataChanged(const GenericData &data)
    {
        std::cout << "Applying style changes to TPMS display" << std::endl;
        applyStyle(container, data.style);
    }

    /// @brief Applies LVGL styling properties to a given UI object.
    void LVGLHandler::applyStyle(lv_obj_t *obj, const LVGLStyle &style)
    {
        lv_obj_set_style_text_color(obj, style.textColor, 0);
        lv_obj_set_style_bg_color(obj, style.bgColor, 0);
        lv_obj_align(obj, style.alignment, 0, 0);
    }

    /// @brief Handles TPMS event dispatching.
    void LVGLHandler::eventHandler(void *handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
    {
        ESP_LOGI("LCD----------", "! Handler -------------------------------");

        LVGLHandler *handler = static_cast<LVGLHandler *>(handler_arg);

        if (event_id == YABT_EVENT_TPMS)
        {
            ESP_LOGI("LCD----------", "! Handler +++++++++++++++++++");

            yabt::TPMSData *data = static_cast<yabt::TPMSData *>(event_data);
            handler->handleTPMSEvent(*data);
        }
    }

    /// @brief Handles Generic event dispatching.
    void LVGLHandler::genericEventHandler(void *handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
    {
        LVGLHandler *handler = static_cast<LVGLHandler *>(handler_arg);

        // if (event_id == GENERIC_EVENT_ID)
        // {
        //     GenericData *data = static_cast<GenericData *>(event_data);
        //     handler->handleGenericEvent(*data);
        // }
    }
} // namespace lcd
