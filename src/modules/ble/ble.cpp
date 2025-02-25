#include <stdio.h>
#include <string.h>
#include <string>
#include <format>
#include "esp_system.h"
#include "esp_log.h"
#include "esp_event.h"
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_gap_ble_api.h"
// #include "esp_gatts_api.h"
#include "esp_gattc_api.h"

// Теги для логирования
static const char *TAG_BLE = "BLE";
static const char *TAG_BLE_CALLBACK = "BLE_CB";
static const char *TAG_BLE_GATTC = "BLE_GATTC";

void process_ext_adv_report(const esp_ble_gap_ext_adv_report_t &report);
void esp_gap_cb(esp_ble_gap_ext_adv_report_t &report);
std::string process_adv_data(const uint8_t *data, uint8_t data_len, esp_ble_adv_data_type type);

// Pointer to User defined scan_params data structure. This memory space can not be freed until callback of set_scan_params
esp_ble_scan_params_t scan_params = {
    .scan_type = BLE_SCAN_TYPE_PASSIVE,
    .own_addr_type = BLE_ADDR_TYPE_PUBLIC,
    .scan_filter_policy = BLE_SCAN_FILTER_ALLOW_ALL,
    .scan_interval = 0x50,
    .scan_window = 0x30,
    //.scan_duplicate = BLE_SCAN_DUPLICATE_DISABLE
    .scan_duplicate = BLE_SCAN_DUPLICATE_ENABLE};

esp_ble_ext_scan_params_t ext_scan_params = {
    .own_addr_type = BLE_ADDR_TYPE_PUBLIC,
    .filter_policy = BLE_SCAN_FILTER_ALLOW_ALL,
    .scan_duplicate = BLE_SCAN_DUPLICATE_ENABLE,
    .cfg_mask = ESP_BLE_GAP_EXT_SCAN_CFG_UNCODE_MASK | ESP_BLE_GAP_EXT_SCAN_CFG_CODE_MASK,
    .uncoded_cfg = {BLE_SCAN_TYPE_ACTIVE, 40, 40},
    .coded_cfg = {BLE_SCAN_TYPE_ACTIVE, 40, 40},
};

#define GATT_PROFILE_APP_ID 0

// COOLSPO Address
// static esp_bd_addr_t target_addr = {0x52, 0xfe, 0xf4, 0x4c, 0x2f, 0xc3};
static esp_bd_addr_t target_addr = {0xc3, 0x2f, 0x4c, 0xf4, 0xfe, 0x52};
static bool connect = false;
static esp_gatt_if_t gattc_if;

// esp_ble_ext_scan_params_t ext_scan_params = {
//     .own_addr_type = BLE_ADDR_TYPE_PUBLIC,
//     .filter_policy = BLE_SCAN_FILTER_ALLOW_ALL,
//     .scan_duplicate = BLE_SCAN_DUPLICATE_ENABLE,
//     .phy_configs = {
//         { .scan_interval = 0x50, .scan_window = 0x30 }, // PHY 1M для BLE 4.x
//         { .scan_interval = 0x50, .scan_window = 0x30 }, // PHY 2M для BLE 5.0
//         { .scan_interval = 0x50, .scan_window = 0x30 }  // PHY Coded для BLE 5.0
//     }
// };

// ====== BLE FUNCTIONS ======
static void ble_gap_callback(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)
{
    switch (event)
    {
    case ESP_GAP_BLE_SCAN_PARAM_SET_COMPLETE_EVT:
        if (param->scan_param_cmpl.status == ESP_OK)
        {
            ESP_LOGI(TAG_BLE_CALLBACK, "Scan parameters set. Starting scan...");
            esp_ble_gap_start_scanning(0); // 0 = scan indefinitely
        }
        else
        {
            ESP_LOGI(TAG_BLE_CALLBACK, "Failed to set extended scan parameters: %s\n", esp_err_to_name(param->scan_param_cmpl.status));
        }

        break;

    case ESP_GAP_BLE_SET_EXT_SCAN_PARAMS_COMPLETE_EVT:
        if (param->scan_param_cmpl.status == ESP_OK)
        {
            ESP_LOGI(TAG_BLE_CALLBACK, "Scan Ex parameters set. Starting scan...");
            esp_ble_gap_start_ext_scan(0xFFFE, 0); // Начать расширенное сканирование
        }
        else
        {
            ESP_LOGI(TAG_BLE_CALLBACK, "Failed to set extended scan parameters: %s\n", esp_err_to_name(param->scan_param_cmpl.status));
        }

        break;
    case ESP_GAP_BLE_EXT_SCAN_START_COMPLETE_EVT:
        ESP_LOGI(TAG_BLE_CALLBACK, "Started Ex scan...");
        break;

    case ESP_GAP_BLE_EXT_ADV_DATA_SET_COMPLETE_EVT:
        ESP_LOGI(TAG_BLE_CALLBACK, "ESP_GAP_BLE_EXT_ADV_DATA_SET_COMPLETE_EVT ---------------------------------");
        break;

    case ESP_GAP_BLE_SCAN_RESULT_EVT:
    {
        // Too many strings out
        auto scan_result = param->scan_rst;
        if (scan_result.search_evt == ESP_GAP_SEARCH_INQ_RES_EVT)
        {
            ESP_LOGI(TAG_BLE_CALLBACK, "search_evt=ESP_GAP_SEARCH_INQ_RES_EVT  Found device: Addr: %02x:%02x:%02x:%02x:%02x:%02x, RSSI: %d",
                     scan_result.bda[0], scan_result.bda[1], scan_result.bda[2],
                     scan_result.bda[3], scan_result.bda[4], scan_result.bda[5],
                     scan_result.rssi);
        }
        else
        {
            ESP_LOGI(TAG_BLE_CALLBACK, "ESP_GAP_BLE_SCAN_RESULT_EVT search_evt=%i", scan_result.search_evt);
        }
        break;
    }

    case ESP_GAP_BLE_EXT_ADV_REPORT_EVT:
    {
        // esp_ble_gap_ext_adv_report_t &report = param->ext_adv_report.params;
        esp_ble_gap_ext_adv_report_t *report = &param->ext_adv_report.params;
        ESP_LOGI(TAG_BLE_CALLBACK, "Event ===================================================================== ESP_GAP_BLE_EXT_ADV_REPORT_EVT %d", event);
        process_ext_adv_report(*report);
        esp_gap_cb(*report);

        if (memcmp(report->addr, target_addr, ESP_BD_ADDR_LEN) == 0)
        {
            ESP_LOGI(TAG_BLE_CALLBACK, "Found COOSPO H6...");
            connect = true;
            auto ret = esp_ble_gap_stop_ext_scan();
            ESP_LOGI(TAG_BLE_CALLBACK, "Stop scan %s", esp_err_to_name(ret));

            // esp_ble_gattc_open(gattc_if, target_addr, BLE_ADDR_TYPE_RANDOM, true);
        }

        //    uint8_t *adv_name = NULL;
        // uint8_t adv_name_len = 0;
        // uint8_t *adv_name_s = NULL;
        // uint8_t adv_name_s_len = 0;
        //
        // adv_name = esp_ble_resolve_adv_data_by_type(report.adv_data, report.adv_data_len, ESP_BLE_AD_TYPE_NAME_CMPL, &adv_name_len);
        // adv_name_s = esp_ble_resolve_adv_data_by_type(report.adv_data, report.adv_data_len, ESP_BLE_AD_TYPE_NAME_SHORT, &adv_name_s_len);
        // if (adv_name != NULL)
        //{
        //    std::string str((char *)adv_name);
        //    str.resize(adv_name_len);
        //    ESP_LOGI(TAG_BLE_CALLBACK, "Event adv_name long %s len %i", str.c_str(), str.length());
        //}
        // if (adv_name_s != NULL)
        //{
        //    std::string str((char *)adv_name_s);
        //    str.resize(adv_name_s_len);
        //    ESP_LOGI(TAG_BLE_CALLBACK, "Event adv_name short %s len %i", str.c_str(), str.length());
        //}
        //
        // uint8_t *adv_appearance;
        // uint8_t adv_appearance_len = 0;
        //
        // adv_appearance = esp_ble_resolve_adv_data_by_type(report.adv_data, report.adv_data_len, ESP_BLE_AD_TYPE_APPEARANCE, &adv_appearance_len);
        // ESP_LOGI(TAG_BLE_CALLBACK, "Appearance len %hhu", adv_appearance_len);
        //
        // ESP_LOGI(TAG_BLE_CALLBACK, "SID %hhu %02X:%02X:%02X:%02X:%02X:%02X", report.sid, report.addr[0], report.addr[1], report.addr[2], report.addr[3], report.addr[4], report.addr[5]);
    }
    break;

    case ESP_GAP_BLE_SCAN_START_COMPLETE_EVT:
        if (param->scan_start_cmpl.status == ESP_BT_STATUS_SUCCESS)
        {
            ESP_LOGI(TAG_BLE_CALLBACK, "BLE scan started successfully");
        }
        else
        {
            ESP_LOGE(TAG_BLE_CALLBACK, "Failed to start BLE scan");
        }
        break;

    case ESP_GAP_BLE_SCAN_STOP_COMPLETE_EVT:
        ESP_LOGI(TAG_BLE_CALLBACK, "BLE scan stopped");
        if (param->scan_stop_cmpl.status == ESP_BT_STATUS_SUCCESS)
        {
            ESP_LOGI(TAG_BLE_CALLBACK, "Scan stopped");
            // scanning = false;
            //  Попытка подключения после остановки сканирования
            if (gattc_if != ESP_GATT_IF_NONE)
            {
                ESP_LOGI(TAG_BLE_CALLBACK, "Connecting to target device...");
                // esp_ble_gattc_open(gattc_if, target_addr, BLE_ADDR_TYPE_PUBLIC, true);
                auto ret = esp_ble_gattc_open(gattc_if, target_addr, BLE_ADDR_TYPE_RANDOM, true);
                ESP_LOGE(TAG_BLE, "Connecting result: %s", esp_err_to_name(ret));
            }
        }
        else
        {
            ESP_LOGE(TAG_BLE_CALLBACK, "Scan stop failed: %d", param->scan_stop_cmpl.status);
        }
        break;

    case ESP_GAP_BLE_EXT_SCAN_STOP_COMPLETE_EVT:
        ESP_LOGI(TAG_BLE_CALLBACK, "BLE Ext scan stopped");
        if (param->scan_stop_cmpl.status == ESP_BT_STATUS_SUCCESS)
        {
            ESP_LOGI(TAG_BLE_CALLBACK, "Scan Ext stopped");
            // scanning = false;
            //  Попытка подключения после остановки сканирования
            if (gattc_if != ESP_GATT_IF_NONE)
            {
                ESP_LOGI(TAG_BLE_CALLBACK, "Connecting to target device...");
                auto ret = esp_ble_gattc_open(gattc_if, target_addr, BLE_ADDR_TYPE_RANDOM, true);
                ESP_LOGE(TAG_BLE, "Connecting result: %s", esp_err_to_name(ret));
            }
        }
        else
        {
            ESP_LOGE(TAG_BLE_CALLBACK, "Scan stop failed: %d", param->scan_stop_cmpl.status);
        }
        break;

    default:
        ESP_LOGI(TAG_BLE_CALLBACK, "Event in default -------------- %i", (int)event);

        break;
    }
}

// Функция форматирования структуры esp_ble_gap_ext_adv_report_t
void process_ext_adv_report(const esp_ble_gap_ext_adv_report_t &report)
{
    // Форматируем каждое поле
    std::string event_type_str = std::format("Event Type: 0x{:02x}", report.event_type);
    std::string addr_type_str = std::format("Address Type: 0x{:02x}", report.addr_type);
    std::string addr_str = std::format("Address: {:02x}:{:02x}:{:02x}:{:02x}:{:02x}:{:02x}", report.addr[5], report.addr[4], report.addr[3], report.addr[2], report.addr[1], report.addr[0]);
    std::string primary_phy_str = std::format("Primary PHY: 0x{:02x}", report.primary_phy);
    std::string secondary_phy_str = std::format("Secondary PHY: 0x{:02x}", report.secondly_phy);
    std::string sid_str = std::format("SID: {}", report.sid);
    std::string tx_power_str = std::format("Tx Power: {} dBm", report.tx_power);
    std::string rssi_str = std::format("RSSI: {} dBm", report.rssi);
    std::string per_adv_interval_str = std::format("Periodic Adv Interval: {} ms", report.per_adv_interval * 125 / 100);
    std::string dir_addr_type_str = std::format("Direct Address Type: 0x{:02x}", report.dir_addr_type);
    std::string dir_addr_str = std::format("Direct Address: {:02x}:{:02x}:{:02x}:{:02x}:{:02x}:{:02x}", report.dir_addr[5], report.dir_addr[4], report.dir_addr[3], report.dir_addr[2], report.dir_addr[1], report.dir_addr[0]);
    std::string data_status_str = std::format("Data Status: 0x{:02x}", report.data_status);
    std::string adv_data_len_str = std::format("Adv Data Length: {}", report.adv_data_len);

    // Форматируем adv_data как hex-строку
    std::string adv_data_str = "Adv Data: ";
    for (int i = 0; i < report.adv_data_len; i++)
    {
        adv_data_str += std::format("{:02x} ", report.adv_data[i]);
    }

    // Выводим все строки в лог
    ESP_LOGI(TAG_BLE_CALLBACK, "%s", event_type_str.c_str());
    ESP_LOGI(TAG_BLE_CALLBACK, "%s", addr_type_str.c_str());
    ESP_LOGI(TAG_BLE_CALLBACK, "%s", addr_str.c_str());
    ESP_LOGI(TAG_BLE_CALLBACK, "%s", primary_phy_str.c_str());
    ESP_LOGI(TAG_BLE_CALLBACK, "%s", secondary_phy_str.c_str());
    ESP_LOGI(TAG_BLE_CALLBACK, "%s", sid_str.c_str());
    ESP_LOGI(TAG_BLE_CALLBACK, "%s", tx_power_str.c_str());
    ESP_LOGI(TAG_BLE_CALLBACK, "%s", rssi_str.c_str());
    ESP_LOGI(TAG_BLE_CALLBACK, "%s", per_adv_interval_str.c_str());
    ESP_LOGI(TAG_BLE_CALLBACK, "%s", dir_addr_type_str.c_str());
    ESP_LOGI(TAG_BLE_CALLBACK, "%s", dir_addr_str.c_str());
    ESP_LOGI(TAG_BLE_CALLBACK, "%s", data_status_str.c_str());
    ESP_LOGI(TAG_BLE_CALLBACK, "%s", adv_data_len_str.c_str());
    ESP_LOGI(TAG_BLE_CALLBACK, "%s", adv_data_str.c_str());
}

// Обработчик BLE-событий
void esp_gap_cb(esp_ble_gap_ext_adv_report_t &report)
{
    // Список всех типов
    esp_ble_adv_data_type types[] = {
        ESP_BLE_AD_TYPE_FLAG, ESP_BLE_AD_TYPE_16SRV_PART, ESP_BLE_AD_TYPE_16SRV_CMPL,
        ESP_BLE_AD_TYPE_32SRV_PART, ESP_BLE_AD_TYPE_32SRV_CMPL, ESP_BLE_AD_TYPE_128SRV_PART,
        ESP_BLE_AD_TYPE_128SRV_CMPL, ESP_BLE_AD_TYPE_NAME_SHORT, ESP_BLE_AD_TYPE_NAME_CMPL,
        ESP_BLE_AD_TYPE_TX_PWR, ESP_BLE_AD_TYPE_DEV_CLASS, ESP_BLE_AD_TYPE_SM_TK,
        ESP_BLE_AD_TYPE_SM_OOB_FLAG, ESP_BLE_AD_TYPE_INT_RANGE, ESP_BLE_AD_TYPE_SOL_SRV_UUID,
        ESP_BLE_AD_TYPE_128SOL_SRV_UUID, ESP_BLE_AD_TYPE_SERVICE_DATA, ESP_BLE_AD_TYPE_PUBLIC_TARGET,
        ESP_BLE_AD_TYPE_RANDOM_TARGET, ESP_BLE_AD_TYPE_APPEARANCE, ESP_BLE_AD_TYPE_ADV_INT,
        ESP_BLE_AD_TYPE_LE_DEV_ADDR, ESP_BLE_AD_TYPE_LE_ROLE, ESP_BLE_AD_TYPE_SPAIR_C256,
        ESP_BLE_AD_TYPE_SPAIR_R256, ESP_BLE_AD_TYPE_32SOL_SRV_UUID, ESP_BLE_AD_TYPE_32SERVICE_DATA,
        ESP_BLE_AD_TYPE_128SERVICE_DATA,
        // ESP_BLE_AD_TYPE_LE_SECURE_CONFIRM, ESP_BLE_AD_TYPE_LE_SECURE_RANDOM,
        // ESP_BLE_AD_TYPE_URI, ESP_BLE_AD_TYPE_INDOOR_POSITION, ESP_BLE_AD_TYPE_TRANS_DISC_DATA,
        // ESP_BLE_AD_TYPE_LE_SUPPORT_FEATURE, ESP_BLE_AD_TYPE_CHAN_MAP_UPDATE,
        ESP_BLE_AD_MANUFACTURER_SPECIFIC_TYPE};

    int num_types = sizeof(types) / sizeof(types[0]);

    for (int i = 0; i < num_types; i++)
    {
        std::string result = process_adv_data(report.adv_data, report.adv_data_len, types[i]);
        if (result.empty())
            continue;
        ESP_LOGI(TAG_BLE_CALLBACK, "%s", result.c_str());
    }
}

// Функция обработки данных рекламы с возвратом строки
std::string process_adv_data(const uint8_t *data, uint8_t data_len, esp_ble_adv_data_type type)
{
    static std::string empty;
    uint8_t length;
    const uint8_t *value = esp_ble_resolve_adv_data_by_type((uint8_t *)data, data_len, type, &length);

    if (value == NULL || length == 0)
    {
        return empty;
        // return std::format("Type 0x{:02x}: Not present", (int)type);
    }

    switch (type)
    {
    case ESP_BLE_AD_TYPE_FLAG:
        return std::format("Flags: 0x{:02x}", value[0]);
    case ESP_BLE_AD_TYPE_16SRV_PART:
    case ESP_BLE_AD_TYPE_16SRV_CMPL:
    case ESP_BLE_AD_TYPE_SOL_SRV_UUID:
    {
        std::string result = "16-bit UUIDs: ";
        for (int i = 0; i < length; i += 2)
        {
            uint16_t uuid = (value[i + 1] << 8) | value[i];
            result += std::format("0x{:04x} ", uuid);
        }
        return result;
    }
    case ESP_BLE_AD_TYPE_32SRV_PART:
    case ESP_BLE_AD_TYPE_32SRV_CMPL:
    case ESP_BLE_AD_TYPE_32SOL_SRV_UUID:
    {
        std::string result = "32-bit UUIDs: ";
        for (int i = 0; i < length; i += 4)
        {
            uint32_t uuid = (value[i + 3] << 24) | (value[i + 2] << 16) | (value[i + 1] << 8) | value[i];
            result += std::format("0x{:08x} ", uuid);
        }
        return result;
    }
    case ESP_BLE_AD_TYPE_128SRV_PART:
    case ESP_BLE_AD_TYPE_128SRV_CMPL:
    case ESP_BLE_AD_TYPE_128SOL_SRV_UUID:
        return std::format("128-bit UUID: {:02x}{:02x}{:02x}{:02x}-{:02x}{:02x}-{:02x}{:02x}-{:02x}{:02x}-{:02x}{:02x}{:02x}{:02x}{:02x}{:02x}",
                           value[15], value[14], value[13], value[12], value[11], value[10], value[9], value[8],
                           value[7], value[6], value[5], value[4], value[3], value[2], value[1], value[0]);
    case ESP_BLE_AD_TYPE_NAME_SHORT:
    case ESP_BLE_AD_TYPE_NAME_CMPL:
        return std::format("Name: {}", std::string((const char *)value, length));
    case ESP_BLE_AD_TYPE_TX_PWR:
        return std::format("Tx Power: {} dBm", (int8_t)value[0]);
    case ESP_BLE_AD_TYPE_DEV_CLASS:
        return std::format("Device Class: 0x{:02x}{:02x}{:02x}", value[2], value[1], value[0]);
    case ESP_BLE_AD_TYPE_SM_TK:
    {
        std::string result = "SM TK: ";
        for (int i = 0; i < length; i++)
        {
            result += std::format("{:02x} ", value[i]);
        }
        return result;
    }
    case ESP_BLE_AD_TYPE_SM_OOB_FLAG:
        return std::format("SM OOB Flags: 0x{:02x}", value[0]);
    case ESP_BLE_AD_TYPE_INT_RANGE:
        return std::format("Interval Range: Min {}, Max {}",
                           (value[1] << 8) | value[0], (value[3] << 8) | value[2]);
    case ESP_BLE_AD_TYPE_SERVICE_DATA:
    case ESP_BLE_AD_TYPE_32SERVICE_DATA:
    case ESP_BLE_AD_TYPE_128SERVICE_DATA:
    {
        std::string result = "Service Data: ";
        for (int i = 0; i < length; i++)
        {
            result += std::format("{:02x} ", value[i]);
        }
        return result;
    }
    case ESP_BLE_AD_TYPE_PUBLIC_TARGET:
    case ESP_BLE_AD_TYPE_RANDOM_TARGET:
    {
        std::string result = "Target Address: ";
        for (int i = 0; i < length; i += 6)
        {
            result += std::format("{:02x}:{:02x}:{:02x}:{:02x}:{:02x}:{:02x} ",
                                  value[i + 5], value[i + 4], value[i + 3], value[i + 2], value[i + 1], value[i]);
        }
        return result;
    }
    case ESP_BLE_AD_TYPE_APPEARANCE:
        return std::format("Appearance: 0x{:04x}", (value[1] << 8) | value[0]);
    case ESP_BLE_AD_TYPE_ADV_INT:
        return std::format("Adv Interval: {} ms", ((value[1] << 8) | value[0]) * 625 / 1000);
    case ESP_BLE_AD_TYPE_LE_DEV_ADDR:
        return std::format("LE Device Address: {:02x}:{:02x}:{:02x}:{:02x}:{:02x}:{:02x}",
                           value[5], value[4], value[3], value[2], value[1], value[0]);
    case ESP_BLE_AD_TYPE_LE_ROLE:
        return std::format("LE Role: 0x{:02x}", value[0]);
    case ESP_BLE_AD_TYPE_SPAIR_C256:
    case ESP_BLE_AD_TYPE_SPAIR_R256:
    {
        std::string result = type == ESP_BLE_AD_TYPE_SPAIR_C256 ? "Secure Pairing C256: " : "Secure Pairing R256: ";
        for (int i = 0; i < length; i++)
        {
            result += std::format("{:02x} ", value[i]);
        }
        return result;
    }
    case ESP_BLE_AD_TYPE_LE_SECURE_CONFIRM:
    case ESP_BLE_AD_TYPE_LE_SECURE_RANDOM:
    {
        std::string result = type == ESP_BLE_AD_TYPE_LE_SECURE_CONFIRM ? "LE Secure Confirm: " : "LE Secure Random: ";
        for (int i = 0; i < length; i++)
        {
            result += std::format("{:02x} ", value[i]);
        }
        return result;
    }
    case ESP_BLE_AD_TYPE_URI:
        return std::format("URI: {}", std::string((const char *)value, length));
    case ESP_BLE_AD_TYPE_INDOOR_POSITION:
        return std::format("Indoor Positioning: {}", value[0] ? "Enabled" : "Disabled");
    case ESP_BLE_AD_TYPE_TRANS_DISC_DATA:
    {
        std::string result = "Transport Discovery Data: ";
        for (int i = 0; i < length; i++)
        {
            result += std::format("{:02x} ", value[i]);
        }
        return result;
    }
    case ESP_BLE_AD_TYPE_LE_SUPPORT_FEATURE:
    {
        std::string result = "LE Supported Features: ";
        for (int i = 0; i < length; i++)
        {
            result += std::format("{:02x} ", value[i]);
        }
        return result;
    }
    case ESP_BLE_AD_TYPE_CHAN_MAP_UPDATE:
    {
        std::string result = "Channel Map Update: ";
        for (int i = 0; i < length; i++)
        {
            result += std::format("{:02x} ", value[i]);
        }
        return result;
    }
    case ESP_BLE_AD_MANUFACTURER_SPECIFIC_TYPE:
    {
        std::string result = std::format("Manufacturer Data - Company ID: 0x{:04x}, Data: ", (value[1] << 8) | value[0]);
        for (int i = 2; i < length; i++)
        {
            result += std::format("{:02x} ", value[i]);
        }
        return result;
    }
    default:
        std::string result = std::format("Unknown type 0x{:02x}: ", (int)type);
        for (int i = 0; i < length; i++)
        {
            result += std::format("{:02x} ", value[i]);
        }
        return result;
    }
}

// Обработчик GATT-событий
static void esp_gattc_cb(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if_param, esp_ble_gattc_cb_param_t *param)
{
    switch (event)
    {
    case ESP_GATTC_REG_EVT:
        if (param->reg.status == ESP_GATT_OK)
        {
            ESP_LOGI(TAG_BLE_GATTC, "GATT client registered, gattc_if = %d", gattc_if_param);
            gattc_if = gattc_if_param;
            // esp_ble_gap_start_ext_scan(0xFFFFFFFF, 0);
        }
        else
        {
            ESP_LOGE(TAG_BLE_GATTC, "GATT client registration failed: %d", param->reg.status);
        }
        break;
    case ESP_GATTC_OPEN_EVT:
        if (param->open.status == ESP_GATT_OK)
        {
            auto conn_id = param->open.conn_id;
            ESP_LOGI(TAG_BLE_GATTC, "Connected to COOSPO H6, conn_id = %d", conn_id);
            esp_ble_gattc_search_service(gattc_if, conn_id, NULL); // Поиск всех сервисов
        }
        else
        {
            ESP_LOGE(TAG_BLE_GATTC, "Connection failed, status = %d", param->open.status);
        }
        break;
    case ESP_GATTC_SEARCH_RES_EVT:
    {
        auto search_res = &param->search_res;
        ESP_LOGI(TAG_BLE_GATTC, "Service found: UUID len: %d, Start Handle: %d, End Handle: %d", search_res->srvc_id.uuid.len, search_res->start_handle, search_res->end_handle);

        if (search_res->srvc_id.uuid.len == ESP_UUID_LEN_16)
        {
            if (search_res->srvc_id.uuid.uuid.uuid16 == 0x180D)
            {
                ESP_LOGI(TAG_BLE_GATTC, "Found Heart Rate Service (0x180D), handle range: %d-%d", search_res->start_handle, search_res->end_handle);
            }
            else if (search_res->srvc_id.uuid.uuid.uuid16 == 0x180F)
            {
                ESP_LOGI(TAG_BLE_GATTC, "Found Battery Service (0x180F), handle range: %d-%d", search_res->start_handle, search_res->end_handle);
            }
        }
        break;
    }
    case ESP_GATTC_SEARCH_CMPL_EVT:
        if (param->search_cmpl.status == ESP_GATT_OK)
        {
            ESP_LOGI(TAG_BLE_GATTC, "Service discovery completed, conn_id = %d", param->search_cmpl.conn_id);
        }
        else
        {
            ESP_LOGE(TAG_BLE_GATTC, "Service discovery failed: %d", param->search_cmpl.status);
        }
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
    esp_ble_gattc_register_callback(esp_gattc_cb);
    esp_ble_gattc_app_register(GATT_PROFILE_APP_ID);

    // https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/bluetooth/esp_gap_ble.html#_CPPv427esp_ble_gap_set_scan_paramsP21esp_ble_scan_params_t
    // ret = esp_ble_gap_set_scan_params(&scan_params);
    // if (ret != ESP_OK)
    //{
    //    ESP_LOGE(TAG_BLE, "Failed esp_ble_gap_set_scan_params: %s", esp_err_to_name(ret));
    //    return;
    //}

    ret = esp_ble_gap_set_ext_scan_params(&ext_scan_params);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG_BLE, "Failed esp_ble_gap_set_ext_scan_params: %s", esp_err_to_name(ret));
        return;
    }

    // ret = esp_ble_gap_start_ext_scan(0xFFFE, 0);
    // if (ret != ESP_OK)
    //{
    //     ESP_LOGE(TAG_BLE, "Failed esp_ble_gap_start_ext_scan: %s", esp_err_to_name(ret));
    //     return;
    // }

    // esp_ble_gattc_open(gattc_if, target_addr, BLE_ADDR_TYPE_RANDOM, true);

    // esp_ble_gap_set_scan_params(&((esp_ble_scan_params_t)
    //{
    //     .scan_type = BLE_SCAN_TYPE_PASSIVE,
    //     .own_addr_type = BLE_ADDR_TYPE_PUBLIC,
    //     .scan_filter_policy = BLE_SCAN_FILTER_ALLOW_ALL,
    //     .scan_interval = 0x50,
    //     .scan_window = 0x30}));

    ESP_LOGI(TAG_BLE, "BLE initialized.");
}
