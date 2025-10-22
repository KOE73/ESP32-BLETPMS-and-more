// LCD Event Handling - Implementation File
// Implements event-driven data updates for LVGL UI components

#include "../lvgl_async_helper.hpp"

#include "lvgl_tpms.hpp"

#include <nlohmann/json.hpp>
using nlohmann::json;

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
        lv_obj_set_width(ui_Panel, 100);
        lv_obj_set_height(ui_Panel, 80);

        lv_style_init(&style);

        lv_obj_set_style_pad_all(ui_Panel, 0, LV_PART_MAIN);

        lv_obj_remove_flag(ui_Panel, LV_OBJ_FLAG_SCROLLABLE); /// Flags
        lv_obj_set_style_radius(ui_Panel, 20, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_color(ui_Panel, lv_color_hex(0x001020), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_opa(ui_Panel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_border_color(ui_Panel, lv_color_hex(0x002030), LV_PART_MAIN | LV_STATE_DEFAULT);
        // lv_obj_set_style_border_opa(ui_Panel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_border_width(ui_Panel, 3, LV_PART_MAIN | LV_STATE_DEFAULT);
        // lv_obj_set_style_shadow_color(ui_Panel, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
        // lv_obj_set_style_shadow_opa(ui_Panel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
        // lv_obj_set_style_shadow_width(ui_Panel, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
        // lv_obj_set_style_shadow_spread(ui_Panel, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
        // lv_obj_set_style_shadow_offset_x(ui_Panel, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
        // lv_obj_set_style_shadow_offset_y(ui_Panel, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_color(ui_Panel, lv_color_hex(0x1ECDCF), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_opa(ui_Panel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

        pressureLabel = lv_label_create(ui_Panel);
        lv_label_set_text(pressureLabel, "0.0");
        lv_obj_align(pressureLabel, LV_ALIGN_TOP_MID, 0, 0);
        lv_obj_set_size(pressureLabel, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
        // lv_style_set_text_font(&style, &lv_font_montserrat_24);

        // lv_obj_set_width(pressureLabel, LV_SIZE_CONTENT);  /// 1
        // lv_obj_set_height(pressureLabel, LV_SIZE_CONTENT); /// 1
        // lv_obj_set_x(pressureLabel, -174);
        // lv_obj_set_y(pressureLabel, -35);
        // lv_obj_set_align(pressureLabel, LV_ALIGN_CENTER);
        // lv_label_set_text(pressureLabel, "text");
        lv_obj_set_style_text_color(pressureLabel, lv_color_hex(0xFF4040), LV_PART_MAIN | LV_STATE_DEFAULT);
        // lv_obj_set_style_text_opa(pressureLabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_font(pressureLabel, &lv_font_montserrat_48, LV_PART_MAIN | LV_STATE_DEFAULT);
        // lv_obj_set_style_outline_color(pressureLabel, lv_color_hex(0xE3D5D5), LV_PART_MAIN | LV_STATE_DEFAULT);
        // lv_obj_set_style_outline_opa(pressureLabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
        // lv_obj_set_style_outline_width(pressureLabel, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
        // lv_obj_set_style_outline_pad(pressureLabel, 5, LV_PART_MAIN | LV_STATE_DEFAULT);

        temperatureLabel = lv_label_create(ui_Panel);
        lv_label_set_text(temperatureLabel, "-- °C");
        lv_obj_align(temperatureLabel, LV_ALIGN_BOTTOM_MID, 0, -5);
        lv_obj_set_size(temperatureLabel, LV_SIZE_CONTENT, 20);
        lv_obj_set_style_text_color(temperatureLabel, lv_color_hex(0xFF4040), LV_PART_MAIN | LV_STATE_DEFAULT);

        lv_obj_set_style_text_font(temperatureLabel, &lv_font_montserrat_24, LV_PART_MAIN | LV_STATE_DEFAULT);

        lv_obj_invalidate(container);
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
        ESP_LOGI("LCD----------", "TPMS updated: %.1f Psi, %.1f °C", data.pressure_Psi, data.temperatureC);

        // Prepare JSON object (future CBOR-compatible structure)
        json j = {
            {"pressure_Psi", data.pressure_Psi},
            {"temperature_C", data.temperatureC},
            {"battery", data.batteryPercentage},
            {"alarm", data.alarmFlag}};

        // Copy JSON to heap (needed because lv_async_call executes later)

        // auto *payload = new json(j);
        auto cbor = json::to_cbor(j);
        auto payload = std::vector<uint8_t>(std::move(cbor));
        // auto *payload = new std::vector<uint8_t>(std::move(cbor));

        // lv_label_set_text_fmt(pressureLabel, "%.1f", data.pressure_Psi);
        // lv_label_set_text_fmt(temperatureLabel, "%.1f °C", data.temperatureC);

        // Schedule GUI update on LVGL thread

        lvgl::post([this, payload]()
                   {
                //std::unique_ptr<json> jptr(static_cast<json*>(param)); // auto-delete at the end
                //const json &j = *jptr;
                //std::unique_ptr<std::vector<uint8_t>> buf(static_cast<std::vector<uint8_t>*>(param));
                auto j = json::from_cbor(payload);

                float psi = j.value("pressure_Psi", 0.0f);
                float temp = j.value("temperature_C", 0.0f);

                ESP_LOGI("LCD----------", "Parsed CBOR: psi = %.1f, temp = %.1f °C", psi, temp);


                lv_label_set_text_fmt(pressureLabel, "%.1f", psi);
                lv_label_set_text_fmt(temperatureLabel, "%.1f °C", temp);

                lv_obj_invalidate(pressureLabel);
                lv_obj_invalidate(temperatureLabel);

                lv_obj_mark_layout_as_dirty(pressureLabel);
                lv_obj_mark_layout_as_dirty(temperatureLabel);
                lv_refr_now(NULL);

                ESP_LOGI("LCD----------", "Labels updated via CBOR/JSON payload"); });

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
