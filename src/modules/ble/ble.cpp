#include <stdio.h>
#include <string.h>
#include <string>
#include "esp_system.h"
#include "esp_log.h"
#include "esp_event.h"
#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_bt.h"
#include "esp_bt_main.h"

// Теги для логирования
static const char *TAG_BLE = "BLE";

// Pointer to User defined scan_params data structure. This memory space can not be freed until callback of set_scan_params
esp_ble_scan_params_t scan_params = {
    .scan_type = BLE_SCAN_TYPE_PASSIVE,
    .own_addr_type = BLE_ADDR_TYPE_PUBLIC,
    .scan_filter_policy = BLE_SCAN_FILTER_ALLOW_ALL,
    .scan_interval = 0x50,
    .scan_window = 0x30,
    .scan_duplicate = BLE_SCAN_DUPLICATE_DISABLE};

// ====== BLE FUNCTIONS ======
static void ble_gap_callback(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)
{
    switch (event)
    {
    case ESP_GAP_BLE_SCAN_PARAM_SET_COMPLETE_EVT:
        ESP_LOGI(TAG_BLE, "Scan parameters set. Starting scan...");
        esp_ble_gap_start_scanning(0); // 0 = scan indefinitely
        break;

    case ESP_GAP_BLE_SCAN_RESULT_EVT:
    {
        // Too many strings out
        auto scan_result = param->scan_rst;
        //if (scan_result.search_evt == ESP_GAP_SEARCH_INQ_RES_EVT)
        //{
        //    ESP_LOGI(TAG_BLE, "Found device: Addr: %02x:%02x:%02x:%02x:%02x:%02x, RSSI: %d",
        //             scan_result.bda[0], scan_result.bda[1], scan_result.bda[2],
        //             scan_result.bda[3], scan_result.bda[4], scan_result.bda[5],
        //             scan_result.rssi);
        //}
        break;
    }

    case ESP_GAP_BLE_EXT_ADV_REPORT_EVT:
    {
        auto p = param->ext_adv_report.params;
        // ESP_LOGI(TAG_BLE, "Event ESP_GAP_BLE_EXT_ADV_REPORT_EVT %d", event);
        uint8_t *adv_name = NULL;
        uint8_t adv_name_len = 0;
        uint8_t *adv_name_s = NULL;
        uint8_t adv_name_s_len = 0;

        adv_name = esp_ble_resolve_adv_data_by_type(p.adv_data, p.adv_data_len, ESP_BLE_AD_TYPE_NAME_CMPL, &adv_name_len);
        adv_name_s = esp_ble_resolve_adv_data_by_type(p.adv_data, p.adv_data_len, ESP_BLE_AD_TYPE_NAME_SHORT, &adv_name_s_len);
        if (adv_name != NULL)
        {
            std::string str((char *)adv_name);
            str.resize(adv_name_len);
            ESP_LOGI(TAG_BLE, "Event adv_name long %s len %i", str.c_str(), str.length());
        }
        if (adv_name_s != NULL)
        {
            std::string str((char *)adv_name_s);
            str.resize(adv_name_s_len);
            ESP_LOGI(TAG_BLE, "Event adv_name short %s len %i", str.c_str(), str.length());
        }

        uint8_t *adv_appearance;
        uint8_t adv_appearance_len = 0;

        adv_appearance = esp_ble_resolve_adv_data_by_type(p.adv_data, p.adv_data_len, ESP_BLE_AD_TYPE_APPEARANCE, &adv_appearance_len);
        ESP_LOGI(TAG_BLE, "Appearance len %hhu", adv_appearance_len);

        ESP_LOGI(TAG_BLE, "SID %hhu %02X:%02X:%02X:%02X:%02X:%02X", p.sid, p.addr[0], p.addr[1], p.addr[2], p.addr[3], p.addr[4], p.addr[5]);
    }
    break;

    case ESP_GAP_BLE_SCAN_START_COMPLETE_EVT:
        if (param->scan_start_cmpl.status == ESP_BT_STATUS_SUCCESS)
        {
            ESP_LOGI(TAG_BLE, "BLE scan started successfully");
        }
        else
        {
            ESP_LOGE(TAG_BLE, "Failed to start BLE scan");
        }
        break;

    case ESP_GAP_BLE_SCAN_STOP_COMPLETE_EVT:
        ESP_LOGI(TAG_BLE, "BLE scan stopped");
        break;

    default:
        break;
    }
}

void ble_init(void)
{
    ESP_LOGI(TAG_BLE, "Initializing BLE...");
    esp_err_t ret;

    // Need call or skip?
    // esp_err_t ret = esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT);
    // if (ret != ESP_OK)
    //{
    //    ESP_LOGE(TAG_BLE, "Failed to release classic BT memory: %s", esp_err_to_name(ret));
    //    return;
    //}

    // https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/bluetooth/controller_vhci.html#_CPPv422esp_bt_controller_initP26esp_bt_controller_config_t
    // https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/bluetooth/controller_vhci.html#_CPPv426esp_bt_controller_config_t
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ret = esp_bt_controller_init(&bt_cfg);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG_BLE, "Bluetooth controller initialization failed: %s", esp_err_to_name(ret));
        return;
    }

    // https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/bluetooth/controller_vhci.html#_CPPv424esp_bt_controller_enable13esp_bt_mode_t
    // https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/bluetooth/controller_vhci.html#_CPPv413esp_bt_mode_t
    // try select from controller type
    // ret = esp_bt_controller_enable(ESP_BT_MODE_BTDM); // ESP_BT_MODE_BLE ESP_BT_MODE_BTDM
    ret = esp_bt_controller_enable(ESP_BT_MODE_BLE); // ESP_BT_MODE_BLE ESP_BT_MODE_BTDM
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG_BLE, "Failed to enable BLE: %s", esp_err_to_name(ret));
        return;
    }

    ret = esp_bluedroid_init();
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG_BLE, "Failed to initialize bluedroid: %s", esp_err_to_name(ret));
        return;
    }

    ret = esp_bluedroid_enable();
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG_BLE, "Failed to enable bluedroid: %s", esp_err_to_name(ret));
        return;
    }

    ret = esp_ble_gap_register_callback(ble_gap_callback);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG_BLE, "Failed esp_ble_gap_register_callback: %s", esp_err_to_name(ret));
        return;
    }

    // https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/bluetooth/esp_gap_ble.html#_CPPv427esp_ble_gap_set_scan_paramsP21esp_ble_scan_params_t
    ret = esp_ble_gap_set_scan_params(&scan_params);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG_BLE, "Failed esp_ble_gap_set_scan_params: %s", esp_err_to_name(ret));
        return;
    }

    // esp_ble_gap_set_scan_params(&((esp_ble_scan_params_t)
    //{
    //     .scan_type = BLE_SCAN_TYPE_PASSIVE,
    //     .own_addr_type = BLE_ADDR_TYPE_PUBLIC,
    //     .scan_filter_policy = BLE_SCAN_FILTER_ALLOW_ALL,
    //     .scan_interval = 0x50,
    //     .scan_window = 0x30}));

    ESP_LOGI(TAG_BLE, "BLE initialized.");
}
