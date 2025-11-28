// LCD Event Handling - Implementation File
// Implements event-driven data updates for LVGL UI components

#include "../lvgl_async_helper.hpp"

#include "lvgl_network.hpp"

__attribute__((weak)) esp_event_loop_handle_t CBOR_loop = NULL;
__attribute__((weak)) esp_event_base_t CBOR_EVENT = NULL;

#include <nlohmann/json.hpp>
using nlohmann::json;

namespace lcd
{
    void cbor_handler_any(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
    {
        ESP_LOGI("LCD----------", "Receive CBOR");
        size_t len = (size_t)event_id;
        uint8_t *buf = (uint8_t *)event_data;
        ESP_LOGI("LCD----------", "CBOR data size: %u bytes", len);

        ((LVGLHandler_Network *)arg)->handleCBOREvent(buf, len);
    }

    /// @brief Constructor initializes the LVGL handler and creates UI labels.
    /// @param parentContainer The parent LVGL container where elements will be placed.
    LVGLHandler_Network::LVGLHandler_Network(lv_obj_t *parentContainer) : container(parentContainer)
    {
        ESP_LOGI("LCD----------", "! LVGLHandler_Network::LVGLHandler_Network");

        createLabels();
        ESP_LOGI("LCD----------", "! LVGLHandler_Network::LVGLHandler_Network 2");

        if (CBOR_loop != NULL)
        {
            ESP_LOGI("LCD----------", "esp_event_handler_register_with . %s", CBOR_EVENT);
            esp_event_handler_register_with(CBOR_loop, CBOR_EVENT, ESP_EVENT_ANY_ID, cbor_handler_any, this);
        }
    }

    /// @brief Creates and configures labels inside the provided container.
    void LVGLHandler_Network::createLabels()
    {
        ESP_LOGI("LCD----------", "! Make label");
        ui_Panel = lv_obj_create(container);
        lv_obj_set_width(ui_Panel, 200);
        lv_obj_set_height(ui_Panel, 80);

        lv_style_init(&style);

        lv_obj_set_style_pad_all(ui_Panel, 0, LV_PART_MAIN);

        lv_obj_remove_flag(ui_Panel, LV_OBJ_FLAG_SCROLLABLE); /// Flags
        lv_obj_set_style_radius(ui_Panel, 20, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_color(ui_Panel, lv_color_hex(0x001020), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_opa(ui_Panel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_border_color(ui_Panel, lv_color_hex(0x002030), LV_PART_MAIN | LV_STATE_DEFAULT);

        lv_obj_set_style_border_width(ui_Panel, 3, LV_PART_MAIN | LV_STATE_DEFAULT);

        lv_obj_set_style_text_color(ui_Panel, lv_color_hex(0x1ECDCF), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_opa(ui_Panel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

        pressureLabel = lv_label_create(ui_Panel);
        lv_label_set_text(pressureLabel, "-.-.-.-");
        lv_obj_align(pressureLabel, LV_ALIGN_TOP_MID, 0, 0);
        lv_obj_set_size(pressureLabel, LV_SIZE_CONTENT, LV_SIZE_CONTENT);

        lv_obj_set_style_text_color(pressureLabel, lv_color_hex(0xFF4040), LV_PART_MAIN | LV_STATE_DEFAULT);

        lv_obj_set_style_text_font(pressureLabel, &lv_font_montserrat_24, LV_PART_MAIN | LV_STATE_DEFAULT);

        lv_obj_invalidate(container);
    }

    /// @brief Handles TPMS event reception and updates UI labels.
    void LVGLHandler_Network::handleCBOREvent(const uint8_t *message, size_t len)
    {
        ESP_LOGI("LCD----------", "LVGLHandler_Network::handleCBOREvent 0. %s", CBOR_EVENT);

        auto payload = std::vector<uint8_t>(message, message + len);
        json j = json::from_cbor(payload);

        ESP_LOGI("LCD----------", "LVGLHandler_Network::handleCBOREvent JSON. %s", j.dump().c_str());

        if (j.contains("cbor") && j["cbor"] == "IP")
        {
            ESP_LOGI("LCD----------", "LVGLHandler_Network::handleCBOREvent IP. %s", CBOR_EVENT);
            lvgl::post([this, payload]()
                       {
                auto j = json::from_cbor(payload);
                lv_label_set_text_fmt(pressureLabel, "%s", j["IP"] .get<std::string>().c_str()); });
        }
        ESP_LOGI("LCD----------", "! Chenged label");
    }

    /// @brief Applies LVGL styling properties to a given UI object.
    void LVGLHandler_Network::applyStyle(lv_obj_t *obj, const LVGLStyle &style)
    {
        lv_obj_set_style_text_color(obj, style.textColor, 0);
        lv_obj_set_style_bg_color(obj, style.bgColor, 0);
        lv_obj_align(obj, style.alignment, 0, 0);
    }

} // namespace lcd
