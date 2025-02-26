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
static void connectHR();
std::string process_adv_data(const uint8_t *data, uint8_t data_len, esp_ble_adv_data_type type);

// Pointer to User defined scan_params data structure. This memory space can not be freed until callback of set_scan_params
esp_ble_scan_params_t scan_params = {
    .scan_type = BLE_SCAN_TYPE_PASSIVE,              /* BLE_SCAN_TYPE_PASSIVE BLE_SCAN_TYPE_ACTIVE */
    .own_addr_type = BLE_ADDR_TYPE_PUBLIC,           /* BLE_ADDR_TYPE_PUBLIC BLE_ADDR_TYPE_RANDOM BLE_ADDR_TYPE_RPA_PUBLIC BLE_ADDR_TYPE_RPA_RANDOM */
    .scan_filter_policy = BLE_SCAN_FILTER_ALLOW_ALL, /* BLE_SCAN_FILTER_ALLOW_ALL BLE_SCAN_FILTER_ALLOW_ONLY_WLST BLE_SCAN_FILTER_ALLOW_UND_RPA_DIR BLE_SCAN_FILTER_ALLOW_WLIST_RPA_DIR */
    .scan_interval = 0x50,                           /*  */
    .scan_window = 0x30,                             /*  */
    .scan_duplicate = BLE_SCAN_DUPLICATE_ENABLE      /* BLE_SCAN_DUPLICATE_DISABLE BLE_SCAN_DUPLICATE_ENABLE (BLE5)BLE_SCAN_DUPLICATE_ENABLE_RESET*/
};

esp_ble_ext_scan_params_t ext_scan_params = {
    .own_addr_type = BLE_ADDR_TYPE_PUBLIC,                                                 /* BLE_ADDR_TYPE_PUBLIC BLE_ADDR_TYPE_RANDOM BLE_ADDR_TYPE_RPA_PUBLIC BLE_ADDR_TYPE_RPA_RANDOM */
    .filter_policy = BLE_SCAN_FILTER_ALLOW_ALL,                                            /* BLE_SCAN_FILTER_ALLOW_ALL BLE_SCAN_FILTER_ALLOW_ONLY_WLST BLE_SCAN_FILTER_ALLOW_UND_RPA_DIR BLE_SCAN_FILTER_ALLOW_WLIST_RPA_DIR */
    .scan_duplicate = BLE_SCAN_DUPLICATE_ENABLE,                                          /* BLE_SCAN_DUPLICATE_DISABLE BLE_SCAN_DUPLICATE_ENABLE (BLE5)BLE_SCAN_DUPLICATE_ENABLE_RESET*/
    .cfg_mask = ESP_BLE_GAP_EXT_SCAN_CFG_UNCODE_MASK | ESP_BLE_GAP_EXT_SCAN_CFG_CODE_MASK, /* Scan Advertisements on the LE1M PHY | on the LE coded PHY */
    .uncoded_cfg = {BLE_SCAN_TYPE_ACTIVE, 40, 40},
    .coded_cfg = {BLE_SCAN_TYPE_ACTIVE, 40, 40},
};

#define GATT_PROFILE_APP_ID 0

static bool connect = false;
static esp_gatt_if_t gattc_if = ESP_GATT_IF_NONE;
// COOLSPO Address
static esp_bd_addr_t target_addr = {0xc3, 0x2f, 0x4c, 0xf4, 0xfe, 0x52};
// static esp_bd_addr_t target_addr = {0x52, 0xfe, 0xf4, 0x4c, 0x2f, 0xc3};
static uint16_t conn_id = 0;
static uint16_t hr_handle = 0; // Дескриптор характеристики пульса

// 13 ff 00 01 82 ea ca 30 07 27 64 8e 03 00 75 09 00 00 42 00
// 
// 13 → Длина (19 байт)
// ff → Тип: Manufacturer Specific Data (ESP_BLE_AD_TYPE_MANUFACTURER_SPECIFIC)
// 00 01 → Производитель (возможно, ID производителя)
// 82 ea ca 30 07 27 64 8e 03 00 75 09 00 00 42 00 → Данные производителя
// https://github.com/ra6070/BLE-TPMS/blob/master/tpms.ino
static esp_bd_addr_t target_addr_TPMS = {0x82, 0xea, 0xca, 0x30, 0x07, 0x27};

// ====== BLE FUNCTIONS ======

static bool ble_gap_callback_legacy(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)
{
    switch (event)
    {
    case ESP_GAP_BLE_SCAN_PARAM_SET_COMPLETE_EVT:
        if (param->scan_param_cmpl.status == ESP_BT_STATUS_SUCCESS)
        {
            ESP_LOGI(TAG_BLE_CALLBACK, "Scan parameters set. Starting scan...");
#ifdef CONFIG_BT_BLE_42_FEATURES_SUPPORTED
            esp_ble_gap_start_scanning(30); // 0 = scan indefinitely
#endif
        }
        else
        {
            ESP_LOGI(TAG_BLE_CALLBACK, "Failed to set extended scan parameters: %s\n", esp_err_to_name(param->scan_param_cmpl.status));
        }
        return true;

    case ESP_GAP_BLE_SCAN_START_COMPLETE_EVT:
        if (param->scan_start_cmpl.status == ESP_BT_STATUS_SUCCESS)
        {
            ESP_LOGI(TAG_BLE_CALLBACK, "BLE scan started successfully");
        }
        else
        {
            ESP_LOGE(TAG_BLE_CALLBACK, "Failed to start BLE scan %i", (int)param->scan_start_cmpl.status);
        }
        return true;

    case ESP_GAP_BLE_SCAN_RESULT_EVT:
        switch (param->scan_rst.search_evt)
        {

        case ESP_GAP_SEARCH_INQ_RES_EVT: /*!< Inquiry result for a peer device. */
            ESP_LOGI(TAG_BLE_CALLBACK, "ESP_GAP_SEARCH_INQ_RES_EVT Device found: %02x:%02x:%02x:%02x:%02x:%02x, RSSI: %d", ESP_BD_ADDR_HEX(param->scan_rst.bda), param->scan_rst.rssi);
            ESP_LOG_BUFFER_HEX(TAG_BLE_CALLBACK, param->scan_rst.ble_adv, param->scan_rst.adv_data_len);
            break;
        case ESP_GAP_SEARCH_INQ_CMPL_EVT: /*!< Inquiry complete. */
            ESP_LOGI(TAG_BLE_CALLBACK, "ESP_GAP_SEARCH_INQ_CMPL_EVT Scan completed");
            break;
        case ESP_GAP_SEARCH_DISC_RES_EVT: /*!< Discovery result for a peer device. */
            ESP_LOGI(TAG_BLE_CALLBACK, "ESP_GAP_SEARCH_DISC_RES_EVT Discovery result for a peer device");
            break;
        case ESP_GAP_SEARCH_DISC_BLE_RES_EVT: /*!< Discovery result for BLE GATT based service on a peer device. */
            ESP_LOGI(TAG_BLE_CALLBACK, "ESP_GAP_SEARCH_DISC_BLE_RES_EVT Discovery result for BLE GATT based service on a peer device");
            break;
        case ESP_GAP_SEARCH_DISC_CMPL_EVT: /*!< Discovery complete. */
            ESP_LOGI(TAG_BLE_CALLBACK, "ESP_GAP_SEARCH_DISC_CMPL_EVT Discovery complete");
            break;
        case ESP_GAP_SEARCH_DI_DISC_CMPL_EVT: /*!< Discovery complete. */
            ESP_LOGI(TAG_BLE_CALLBACK, "ESP_GAP_SEARCH_DI_DISC_CMPL_EVT Discovery complete");
            break;
        case ESP_GAP_SEARCH_SEARCH_CANCEL_CMPL_EVT: /*!< Search cancelled */
            ESP_LOGI(TAG_BLE_CALLBACK, "ESP_GAP_SEARCH_SEARCH_CANCEL_CMPL_EVT Search cancelled");
            break;
        case ESP_GAP_SEARCH_INQ_DISCARD_NUM_EVT: /*!< The number of pkt discarded by flow control */
            ESP_LOGI(TAG_BLE_CALLBACK, "ESP_GAP_SEARCH_INQ_DISCARD_NUM_EVT The number of pkt discarded by flow control");
            break;
        }

        return true;

    case ESP_GAP_BLE_SCAN_STOP_COMPLETE_EVT:
        if (param->scan_stop_cmpl.status == ESP_BT_STATUS_SUCCESS)
        {
            ESP_LOGI(TAG_BLE_CALLBACK, "Scan stopped");
            // scanning = false;
            //  Попытка подключения после остановки сканирования
            if (gattc_if != ESP_GATT_IF_NONE)
            {
                ESP_LOGI(TAG_BLE_CALLBACK, "Legacy Connecting to target device...");
#ifdef CONFIG_BT_BLE_42_FEATURES_SUPPORTED
                auto ret = esp_ble_gattc_open(gattc_if, target_addr, BLE_ADDR_TYPE_RANDOM, true);
                ESP_LOGE(TAG_BLE, "Connecting result: %s", esp_err_to_name(ret));
#endif
            }
        }
        else
        {
            ESP_LOGE(TAG_BLE_CALLBACK, "Scan stop failed: %d", param->scan_stop_cmpl.status);
        }
        return true;

    default:
        return false;
    }
}

static bool ble_gap_callback_ext(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)
{
    switch (event)
    {
    case ESP_GAP_BLE_SET_EXT_SCAN_PARAMS_COMPLETE_EVT:
        if (param->set_ext_scan_params.status == ESP_BT_STATUS_SUCCESS)
        {
            ESP_LOGI(TAG_BLE_CALLBACK, "Extended scan params set, starting scan...");
            esp_ble_gap_start_ext_scan(30000, 1000); // Бесконечное сканирование
        }
        return true;

    case ESP_GAP_BLE_EXT_SCAN_START_COMPLETE_EVT:
        if (param->ext_scan_start.status == ESP_BT_STATUS_SUCCESS)
        {
            ESP_LOGI(TAG_BLE_CALLBACK, "Extended scan started");
        }
        else
        {
            ESP_LOGI(TAG_BLE_CALLBACK, "Extended scan started failed %i", (int)param->ext_scan_start.status);
        }
        return true;

    case ESP_GAP_BLE_EXT_ADV_REPORT_EVT:
    {
        esp_ble_gap_ext_adv_report_t *report = &param->ext_adv_report.params;

        // if (memcmp(report->addr, target_addr, ESP_BD_ADDR_LEN) == 0)
        //{
        //     ESP_LOGI(TAG_BLE_CALLBACK, "Found COOSPO H6...");
        //
        //    process_ext_adv_report(*report);
        //    esp_gap_cb(*report);
        //    ESP_LOG_BUFFER_HEX(TAG_BLE_CALLBACK, report->adv_data, report->adv_data_len);
        //
        //    auto ret = esp_ble_gap_stop_ext_scan();
        //    ESP_LOGI(TAG_BLE_CALLBACK, "Stop scan %s", esp_err_to_name(ret));
        //}
        // return true;

        ESP_LOGI(TAG_BLE_CALLBACK, "Event ===================================================================== ESP_GAP_BLE_EXT_ADV_REPORT_EVT %d", event);
        process_ext_adv_report(*report);
        esp_gap_cb(*report);
        // ESP_LOG_BUFFER_HEX(TAG_BLE_CALLBACK, report->adv_data, report->adv_data_len);

        //if (memcmp(report->addr, target_addr, ESP_BD_ADDR_LEN) == 0)
        //{
        //    ESP_LOGI(TAG_BLE_CALLBACK, "Found COOSPO H6...");
        //    connect = true;
        //    auto ret = esp_ble_gap_stop_ext_scan();
        //    ESP_LOGI(TAG_BLE_CALLBACK, "Stop scan %s", esp_err_to_name(ret));
        //}

        return true;
    }

    case ESP_GAP_BLE_EXT_SCAN_STOP_COMPLETE_EVT:
    {
        if (param->scan_stop_cmpl.status == ESP_BT_STATUS_SUCCESS)
        {
            ESP_LOGI(TAG_BLE_CALLBACK, "Extended scan stopped");
            // scanning = false;
            //  Попытка подключения после остановки сканирования
            // connectHR();
        }
        else
        {
            ESP_LOGE(TAG_BLE_CALLBACK, "Scan stop failed: %d", param->scan_stop_cmpl.status);
        }
        return true;
    }

    case ESP_GAP_BLE_PERIODIC_ADV_REPORT_EVT:
    {
        esp_ble_gap_periodic_adv_report_t *report = &param->period_adv_report.params;
        ESP_LOGI(TAG_BLE_CALLBACK, "Periodic Adv Report - Sync Handle: %d, RSSI: %d", report->sync_handle, report->rssi);
        ESP_LOG_BUFFER_HEX(TAG_BLE_CALLBACK, report->data, report->data_length);
        return true;
    }
    default:
        return false;
    }
}

static void connectHR()
{ // scanning = false;
    //  Попытка подключения после остановки сканирования
    if (gattc_if != ESP_GATT_IF_NONE)
    {
        ESP_LOGI(TAG_BLE_CALLBACK, "Ex Connecting to target device...");

        // auto ret = esp_ble_gattc_open(gattc_if, target_addr, BLE_ADDR_TYPE_RANDOM, true);
        static esp_ble_conn_params_t p1{
            .scan_interval = 0x80,         /*!< Initial scan interval, in units of 0.625ms, the range is 0x0004(2.5ms) to 0xFFFF(10.24s). */
            .scan_window = 0x30,           /*!< Initial scan window, in units of 0.625ms, the range is 0x0004(2.5ms) to 0xFFFF(10.24s). */
            .interval_min = 0x10,          /*!< Minimum connection interval, in units of 1.25ms, the range is 0x0006(7.5ms) to 0x0C80(4s). */
            .interval_max = 0x500,         /*!< Maximum connection interval, in units of 1.25ms, the range is 0x0006(7.5ms) to 0x0C80(4s). */
            .latency = 0,                  /*!< Connection latency, the range is 0x0000(0) to 0x01F3(499). */
            .supervision_timeout = 0x0280, /*!< Connection supervision timeout, in units of 10ms, the range is from 0x000A(100ms) to 0x0C80(32s). */
            .min_ce_len = 0,               /*!< Minimum connection event length, in units of 0.625ms, setting to 0 for no preferred parameters. */
            .max_ce_len = 0,               /*!< Maximum connection event length, in units of 0.625ms, setting to 0 for no preferred parameters. */
        };
        // Параметры подключения
        static esp_ble_gatt_creat_conn_params_t conn_params = {
            .remote_bda = {0},
            .remote_addr_type = BLE_ADDR_TYPE_RANDOM, // BLE_ADDR_TYPE_PUBLIC, BLE_ADDR_TYPE_RANDOM
            .is_direct = true,                        /*!< Direct connection or background auto connection(by now, background auto connection is not supported */
            .is_aux = false,                          /*!< Set to true for BLE 5.0 or higher to enable auxiliary connections; set to false for BLE 4.2 or lower. */
            .own_addr_type = BLE_ADDR_TYPE_PUBLIC,    /*!< Specifies the address type used in the connection request. Set to 0xFF if the address type is unknown. (esp_ble_addr_type_t)0xFF*/
            .phy_mask = ESP_BLE_PHY_1M_PREF_MASK,     /*!< Indicates which PHY connection parameters will be used. When is_aux is false, only the connection params for 1M PHY can be specified */
            .phy_1m_conn_params = &p1,                /*!< Connection parameters for the LE 1M PHY */
            .phy_2m_conn_params = &p1,                /*!< Connection parameters for the LE 2M PHY */
            .phy_coded_conn_params = &p1              /*!< Connection parameters for the LE Coded PHY */
        };
        memcpy(conn_params.remote_bda, target_addr, ESP_BD_ADDR_LEN);

        // Подключение с помощью esp_ble_gattc_enh_open
        esp_err_t ret = esp_ble_gattc_enh_open(gattc_if, &conn_params);

        esp_ble_gattc_aux_open(gattc_if, target_addr, BLE_ADDR_TYPE_RANDOM, true);
        if (ret == ESP_OK)
        {
            ESP_LOGI(TAG_BLE_CALLBACK, "Enhanced connection request sent: %s", esp_err_to_name(ret));
        }
        else
        {
            ESP_LOGE(TAG_BLE_CALLBACK, "Failed to send connection request: %s", esp_err_to_name(ret));
        }
    }
}

static void ble_gap_callback(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)
{
    if (ble_gap_callback_legacy(event, param))
        return;

    if (ble_gap_callback_ext(event, param))
        return;

    switch (event)
    {

    case ESP_GAP_BLE_EXT_ADV_DATA_SET_COMPLETE_EVT:
        ESP_LOGI(TAG_BLE_CALLBACK, "ESP_GAP_BLE_EXT_ADV_DATA_SET_COMPLETE_EVT ????????????????????");
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
    std::string addr_str = std::format("Address: {:02x}:{:02x}:{:02x}:{:02x}:{:02x}:{:02x}", ESP_BD_ADDR_HEX(report.addr), report.addr[0]);
    std::string primary_phy_str = std::format("Primary PHY: 0x{:02x}", report.primary_phy);
    std::string secondary_phy_str = std::format("Secondary PHY: 0x{:02x}", report.secondly_phy);
    std::string sid_str = std::format("SID: {}", report.sid);
    std::string tx_power_str = std::format("Tx Power: {} dBm", report.tx_power);
    std::string rssi_str = std::format("RSSI: {} dBm", report.rssi);
    std::string per_adv_interval_str = std::format("Periodic Adv Interval: {} ms", report.per_adv_interval * 125 / 100);
    std::string dir_addr_type_str = std::format("Direct Address Type: 0x{:02x}", report.dir_addr_type);
    std::string dir_addr_str = std::format("Direct Address: {:02x}:{:02x}:{:02x}:{:02x}:{:02x}:{:02x}", ESP_BD_ADDR_HEX(report.dir_addr));
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
    //ESP_LOGI(TAG_BLE_CALLBACK, "%s", tx_power_str.c_str());
    ESP_LOGI(TAG_BLE_CALLBACK, "%s", rssi_str.c_str());
    //ESP_LOGI(TAG_BLE_CALLBACK, "%s", per_adv_interval_str.c_str());
    //ESP_LOGI(TAG_BLE_CALLBACK, "%s", dir_addr_type_str.c_str());
    //ESP_LOGI(TAG_BLE_CALLBACK, "%s", dir_addr_str.c_str());
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
    {
        auto val = value[0];
        std::string out;

        // Устройство в ограниченном режиме обнаружения.
        // Это означает, что устройство будет доступно только для некоторых типов соединений.
        if (val & ESP_BLE_ADV_FLAG_LIMIT_DISC)
            out.append("ESP_BLE_ADV_FLAG_LIMIT_DISC ");

        // Устройство в общем режиме обнаружения.
        // Устройство будет доступно для обнаружения всеми другими BLE-устройствами.
        if (val & ESP_BLE_ADV_FLAG_GEN_DISC)
            out.append("ESP_BLE_ADV_FLAG_GEN_DISC ");

        // Устройство не поддерживает BR/EDR (Basic Rate / Enhanced Data Rate), то есть это чисто BLE-устройство.
        // Указывает, что устройство не будет работать в классическом Bluetooth-режиме.
        if (val & ESP_BLE_ADV_FLAG_BREDR_NOT_SPT)
            out.append("ESP_BLE_ADV_FLAG_BREDR_NOT_SPT ");

        if (val & ESP_BLE_ADV_FLAG_DMT_CONTROLLER_SPT)
            out.append("ESP_BLE_ADV_FLAG_DMT_CONTROLLER_SPT ");

        if (val & ESP_BLE_ADV_FLAG_DMT_HOST_SPT)
            out.append("ESP_BLE_ADV_FLAG_DMT_HOST_SPT ");

        return std::format("Flags: 0x{:02x} {}", value[0], out);
    }

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
        return std::format("LE Device Address: {:02x}:{:02x}:{:02x}:{:02x}:{:02x}:{:02x}", ESP_BD_ADDR_HEX(value));
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
static void esp_gattc_callback(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if_param, esp_ble_gattc_cb_param_t *param)
{
    switch (event)
    {
    case ESP_GATTC_REG_EVT:
        if (param->reg.status == ESP_GATT_OK)
        {
            ESP_LOGI(TAG_BLE_GATTC, "GATT client registered, gattc_if = %d", gattc_if_param);
            gattc_if = gattc_if_param;
            // esp_ble_gap_start_ext_scan(0xFFFFFFFF, 0);

            connectHR();
        }
        else
        {
            ESP_LOGE(TAG_BLE_GATTC, "GATT client registration failed: %d", param->reg.status);
        }
        break;

    case ESP_GATTC_CONNECT_EVT:
    {
        auto connect = &param->connect;
        ESP_LOGI(TAG_BLE_GATTC, "ESP_GATTC_CONNECT_EVT: " ESP_BD_ADDR_STR, ESP_BD_ADDR_HEX(connect->remote_bda));
        break;
    }

    case ESP_GATTC_DISCONNECT_EVT:
    {
        auto disconnect = &param->disconnect;
        auto ad = disconnect->remote_bda;
        ESP_LOGI(TAG_BLE_GATTC, "ESP_GATTC_DISCONNECT_EVT: " ESP_BD_ADDR_STR " conn_id:%d reason:%d", ESP_BD_ADDR_HEX(ad), disconnect->conn_id, disconnect->reason);
        break;
    }

    case ESP_GATTC_OPEN_EVT:
    {
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
    }

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
    {
        if (param->search_cmpl.status == ESP_GATT_OK)
        {
            ESP_LOGI(TAG_BLE_GATTC, "Service discovery completed, conn_id = %d", param->search_cmpl.conn_id);
        }
        else
        {
            ESP_LOGE(TAG_BLE_GATTC, "Service discovery failed: %d", param->search_cmpl.status);
        }
        break;
    }

    default:
        ESP_LOGE(TAG_BLE_GATTC, "esp_gattc_callback !!! event: %d", (int)event);
        break;
    }
}

void ble_init(void)
{
    ESP_LOGI(TAG_BLE, "Initializing BLE...");
    esp_err_t ret;

    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    if (ESP_ERROR_CHECK_WITHOUT_ABORT(esp_bt_controller_init(&bt_cfg)))
        return;

    if (ESP_ERROR_CHECK_WITHOUT_ABORT(esp_bt_controller_enable(ESP_BT_MODE_BLE)))
        return;

    if (ESP_ERROR_CHECK_WITHOUT_ABORT(esp_bluedroid_init()))
        return;

    if (ESP_ERROR_CHECK_WITHOUT_ABORT(esp_bluedroid_enable()))
        return;

    if (ESP_ERROR_CHECK_WITHOUT_ABORT(esp_ble_gap_register_callback(ble_gap_callback)))
        return;

    if (ESP_ERROR_CHECK_WITHOUT_ABORT(esp_ble_gattc_register_callback(esp_gattc_callback)))
        return;

    if (ESP_ERROR_CHECK_WITHOUT_ABORT(esp_ble_gattc_app_register(GATT_PROFILE_APP_ID)))
        return;

    // Legacy
    // if (ESP_ERROR_CHECK_WITHOUT_ABORT(esp_ble_gap_set_scan_params(&scan_params)))
    //    return;

    // BLE 5.0
     if (ESP_ERROR_CHECK_WITHOUT_ABORT(esp_ble_gap_set_ext_scan_params(&ext_scan_params)))
        return;

    //    connectHR();

    ESP_LOGI(TAG_BLE, "BLE initialized.");
}
