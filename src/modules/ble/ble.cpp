#include <stdio.h>
#include <string.h>

#include <string>
#include <format>
#include <unordered_map>
#include <vector>

#include "esp_system.h"
#include "esp_log.h"
#include "esp_event.h"
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_gap_ble_api.h"
// #include "esp_gatts_api.h"
#include "esp_gattc_api.h"

#include "bluetooth-SIG/assigned_numbers/company_identifiers/company_identifiers.hpp"

#include "ble_debug.hpp"

#include "yabt.hpp"

// Теги для логирования
static const char *TAG_BLE = "BLE";
static const char *TAG_BLE_CALLBACK = "BLE_CALLBACK";
static const char *TAG_BLE_GATTC = "BLE_GATTC";

#define LOG_GATTC_COLOR LOG_ANSI_COLOR_BOLD_BACKGROUND(LOG_COLOR_GREEN, LOG_ANSI_COLOR_BG_YELLOW)

#define INVALID_HANDLE 0

static void connectHR();

// Pointer to User defined scan_params data structure. This memory space can not be freed until callback of set_scan_params
esp_ble_scan_params_t scan_params = {
    .scan_type = BLE_SCAN_TYPE_PASSIVE,              /* BLE_SCAN_TYPE_PASSIVE BLE_SCAN_TYPE_ACTIVE */
    .own_addr_type = BLE_ADDR_TYPE_PUBLIC,           /* BLE_ADDR_TYPE_PUBLIC BLE_ADDR_TYPE_RANDOM BLE_ADDR_TYPE_RPA_PUBLIC BLE_ADDR_TYPE_RPA_RANDOM */
    .scan_filter_policy = BLE_SCAN_FILTER_ALLOW_ALL, /* BLE_SCAN_FILTER_ALLOW_ALL BLE_SCAN_FILTER_ALLOW_ONLY_WLST BLE_SCAN_FILTER_ALLOW_UND_RPA_DIR BLE_SCAN_FILTER_ALLOW_WLIST_RPA_DIR */
    .scan_interval = 0x100,                          /*  */
    .scan_window = 0x100,                            /*  */
    .scan_duplicate = BLE_SCAN_DUPLICATE_DISABLE     /* BLE_SCAN_DUPLICATE_DISABLE BLE_SCAN_DUPLICATE_ENABLE (BLE5)BLE_SCAN_DUPLICATE_ENABLE_RESET*/
};

esp_ble_ext_scan_params_t ext_scan_params = {
    .own_addr_type = BLE_ADDR_TYPE_PUBLIC,                                                 /* BLE_ADDR_TYPE_PUBLIC BLE_ADDR_TYPE_RANDOM BLE_ADDR_TYPE_RPA_PUBLIC BLE_ADDR_TYPE_RPA_RANDOM */
    .filter_policy = BLE_SCAN_FILTER_ALLOW_ALL,                                            /* BLE_SCAN_FILTER_ALLOW_ALL BLE_SCAN_FILTER_ALLOW_ONLY_WLST BLE_SCAN_FILTER_ALLOW_UND_RPA_DIR BLE_SCAN_FILTER_ALLOW_WLIST_RPA_DIR */
    .scan_duplicate = BLE_SCAN_DUPLICATE_DISABLE,                                          /* BLE_SCAN_DUPLICATE_DISABLE BLE_SCAN_DUPLICATE_ENABLE (BLE5)BLE_SCAN_DUPLICATE_ENABLE_RESET*/
    .cfg_mask = ESP_BLE_GAP_EXT_SCAN_CFG_UNCODE_MASK | ESP_BLE_GAP_EXT_SCAN_CFG_CODE_MASK, /* Scan Advertisements on the LE1M PHY | on the LE coded PHY */
    .uncoded_cfg = {BLE_SCAN_TYPE_ACTIVE, 0x100, 0x100},
    .coded_cfg = {BLE_SCAN_TYPE_ACTIVE, 0x100, 0x100},
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

        //
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
        // esp_ble_gap_ext_adv_report_t *report = &param->ext_adv_report.params;

        const yabt::BleGapExtAdvReport Report(param->ext_adv_report.params);
        yabt::BTController::getInstance().GapHanler(Report);

        // return true;

        ESP_LOGI(TAG_BLE_CALLBACK, " ~~~~ %s", Report.getAddr().toString().c_str());
        // ESP_LOG_BUFFER_HEX(TAG_BLE_CALLBACK, param->ext_adv_report.params.adv_data, param->ext_adv_report.params.adv_data_len);

        // ESP_LOGI(TAG_BLE_CALLBACK, " +16BitServiceUUIDs         %s", Report.get16BitServiceUUIDsAsString(false).c_str());
        // ESP_LOGI(TAG_BLE_CALLBACK, " +16BitServiceUUIDs  compl  %s", Report.get16BitServiceUUIDsAsString(true).c_str());
        // ESP_LOGI(TAG_BLE_CALLBACK, " +32BitServiceUUIDs         %s", Report.get32BitServiceUUIDsAsString(false).c_str());
        // ESP_LOGI(TAG_BLE_CALLBACK, " +32BitServiceUUIDs  compl  %s", Report.get32BitServiceUUIDsAsString(true).c_str());
        // ESP_LOGI(TAG_BLE_CALLBACK, " +128BitServiceUUIDs        %s", Report.get128BitServiceUUIDsAsString(false).c_str());
        // ESP_LOGI(TAG_BLE_CALLBACK, " +128BitServiceUUIDs compl  %s", Report.get128BitServiceUUIDsAsString(true).c_str());
        //
        // ESP_LOGI(TAG_BLE_CALLBACK, " +16BitSolServiceUUIDs      %s", Report.get16BitSolServiceUUIDsAsString().c_str());
        // ESP_LOGI(TAG_BLE_CALLBACK, " +128BitSolServiceUUIDs     %s", Report.get128BitSolServiceUUIDsAsString().c_str());

        // ESP_LOGI(TAG_BLE_CALLBACK, " +%s", Report.getMapKeysAsString().c_str());

        // auto flags = Report.getFlags();
        // if (flags.has_value())
        //{
        //     ESP_LOGI(TAG_BLE_CALLBACK, " !!!! Flags:  %x %s", flags.value(), Report.getActiveFlagsDescription().value_or("?").c_str());
        // }
        //
        // auto manufacturerData = Report.getManufacturerData();
        // if (manufacturerData.has_value())
        //{
        //    ESP_LOG_BUFFER_HEX(TAG_BLE_CALLBACK, manufacturerData.value().data(), manufacturerData.value().size_bytes());
        //
        //    auto manufacturerId = Report.getManufacturerId();
        //    ESP_LOGI(TAG_BLE_CALLBACK, " !!!! Id= %d", manufacturerId.value());
        //
        //    auto manufacturerName = Report.getManufacturerName();
        //    ESP_LOGI(TAG_BLE_CALLBACK, " !!!! %s", manufacturerName.value().c_str());
        //}

        auto name1 = Report.getCompleteLocalName();
        if (name1.has_value())
            ESP_LOGI(TAG_BLE_CALLBACK, " ++++ %s", name1.value().c_str());

        // if (memcmp(report->addr, target_addr, ESP_BD_ADDR_LEN) == 0)
        //{
        //      ESP_LOGI(TAG_BLE_CALLBACK, "Found COOSPO H6...");

        //    process_ext_adv_report(*report);
        //    esp_gap_cb(*report);
        //    ESP_LOG_BUFFER_HEX(TAG_BLE_CALLBACK, report->adv_data, report->adv_data_len);

        //    auto ret = esp_ble_gap_stop_ext_scan();
        //    ESP_LOGI(TAG_BLE_CALLBACK, "Stop scan %s", esp_err_to_name(ret));
        //}
        return true;

        // ESP_LOGI(TAG_BLE_CALLBACK, "Event ===================================================================== ESP_GAP_BLE_EXT_ADV_REPORT_EVT %d", event);
        // process_ext_adv_report(*report);
        // esp_gap_cb(*report);
        //  ESP_LOG_BUFFER_HEX(TAG_BLE_CALLBACK, report->adv_data, report->adv_data_len);

        // if (memcmp(report->addr, target_addr, ESP_BD_ADDR_LEN) == 0)
        //{
        //     ESP_LOGI(TAG_BLE_CALLBACK, "Found COOSPO H6...");
        //     connect = true;
        //     auto ret = esp_ble_gap_stop_ext_scan();
        //     ESP_LOGI(TAG_BLE_CALLBACK, "Stop scan %s", esp_err_to_name(ret));
        // }

        return true;
    }

    case ESP_GAP_BLE_EXT_SCAN_STOP_COMPLETE_EVT:
    {
        if (param->scan_stop_cmpl.status == ESP_BT_STATUS_SUCCESS)
        {
            ESP_LOGI(TAG_BLE_CALLBACK, "Extended scan stopped");
            // scanning = false;
            //  Попытка подключения после остановки сканирования
            connectHR();
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

        esp_err_t ret;
        // Подключение с помощью esp_ble_gattc_enh_open
        // esp_err_t ret = esp_ble_gattc_enh_open(gattc_if, &conn_params);

        ret = esp_ble_gattc_aux_open(gattc_if, target_addr, BLE_ADDR_TYPE_RANDOM, true);
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

#pragma region GATCC

struct GATTServiceInfo
{
    uint16_t start_handle;
    uint16_t end_handle;
    esp_bt_uuid_t uuid;
};

// Таблица стандартных сервисов BLE
std::unordered_map<uint16_t, std::string> ble_services = {
    {0x1800, "Generic Access"},
    {0x1801, "Generic Attribute"},
    {0x1802, "Immediate Alert"},
    {0x1803, "Link Loss"},
    {0x1804, "Tx Power"},
    {0x1805, "Current Time Service"},
    {0x1806, "Reference Time Update Service"},
    {0x1807, "Next DST Change Service"},
    {0x1808, "Glucose"},
    {0x1809, "Health Thermometer"},
    {0x180A, "Device Information"},
    {0x180D, "Heart Rate"},
    {0x180E, "Phone Alert Status"},
    {0x180F, "Battery"},
    {0x1810, "Blood Pressure"},
    {0x1811, "Alert Notification Service"},
    {0x1812, "Human Interface Device"},
    {0x1813, "Scan Parameters"},
    {0x1814, "Running Speed and Cadence"},
    {0x1815, "Automation IO"},
    {0x1816, "Cycling Speed and Cadence"},
    {0x1818, "Cycling Power"},
    {0x1819, "Location and Navigation"}};

// Храним найденные сервисы
std::vector<GATTServiceInfo> found_services;

std::string get_service_name(uint16_t uuid16)
{
    auto it = ble_services.find(uuid16);
    if (it != ble_services.end())
    {
        return it->second;
    }
    return std::format("Unknown Service (UUID: 0x{:04X})", uuid16);
}

std::string format_uuid(const esp_bt_uuid_t &uuid)
{
    if (uuid.len == ESP_UUID_LEN_16)
    {
        return std::format("UUID: 0x{:04X} -> {}", uuid.uuid.uuid16, get_service_name(uuid.uuid.uuid16));
    }
    else if (uuid.len == ESP_UUID_LEN_32)
    {
        return std::format("UUID: 0x{:08X}", uuid.uuid.uuid32);
    }
    else
    {
        char uuid_str[37];
        snprintf(uuid_str, sizeof(uuid_str),
                 "%08X-%04X-%04X-%04X-%012X",
                 uuid.uuid.uuid128[0] << 24 | uuid.uuid.uuid128[1] << 16 | uuid.uuid.uuid128[2] << 8 | uuid.uuid.uuid128[3],
                 uuid.uuid.uuid128[4] << 8 | uuid.uuid.uuid128[5],
                 uuid.uuid.uuid128[6] << 8 | uuid.uuid.uuid128[7],
                 uuid.uuid.uuid128[8] << 8 | uuid.uuid.uuid128[9],
                 uuid.uuid.uuid128[10] << 8 | uuid.uuid.uuid128[11]);
        return std::string("Custom Service UUID: ") + uuid_str;
    }
}

void print_found_services()
{
    std::string result = "Found GATT Services:\n";
    for (const auto &service : found_services)
    {
        result += std::format("Start Handle: {}, End Handle: {}, {}\n",
                              service.start_handle,
                              service.end_handle,
                              format_uuid(service.uuid));
    }
    ESP_LOGI(TAG_BLE_GATTC, LOG_GATTC_COLOR "%s" LOG_ANSI_COLOR_RESET, result.c_str());
}

// ============================================================================
// 🧭 GATTC EVENT FLOW — Full Connection Lifecycle
// ----------------------------------------------------------------------------
// This callback (esp_gattc_callback) handles all GATT client events.
// Below is the typical sequence and logic of BLE connection and communication.
//
//  1️⃣ REGISTRATION PHASE
//     - esp_ble_gattc_app_register(app_id)
//         ↓
//     → ESP_GATTC_REG_EVT
//         ⚙️ The app is registered in BLE stack.
//         ⚠️ Save gattc_if (interface handle). You'll need it for all GATT calls.
//
//  2️⃣ CONNECTION PHASE
//     - esp_ble_gattc_open(gattc_if, remote_bda, addr_type, direct)
//         ↓
//     → ESP_GATTC_CONNECT_EVT
//         ⚙️ Physical BLE link established (radio-level connection).
//         ⚠️ Save conn_id and remote_bda.
//         💡 Connection *started successfully*, but not yet confirmed.
//
//         ↓
//     → ESP_GATTC_OPEN_EVT
//         ⚙️ Connection attempt finished (success or failure).
//         ⚠️ Check param->open.status == ESP_GATT_OK.
//         ✅ If success → start service discovery (esp_ble_gattc_search_service)
//
//  3️⃣ SERVICE DISCOVERY PHASE
//     - esp_ble_gattc_search_service(gattc_if, conn_id, NULL)
//         ↓
//     → ESP_GATTC_SEARCH_RES_EVT
//         ⚙️ One service found. Store start_handle, end_handle, and UUID.
//         💡 Called once per service.
//
//         ↓
//     → ESP_GATTC_SEARCH_CMPL_EVT
//         ⚙️ Service discovery complete.
//         ✅ You can now read/write characteristics or enable notifications.
//
//  4️⃣ DATA EXCHANGE PHASE
//     - esp_ble_gattc_read_char() → triggers ESP_GATTC_READ_CHAR_EVT
//         ⚙️ Characteristic value received from server.
//
//     - esp_ble_gattc_write_char() → triggers ESP_GATTC_WRITE_CHAR_EVT
//         ⚙️ Server acknowledged write operation.
//
//     - Notifications/Indications from server → ESP_GATTC_NOTIFY_EVT
//         ⚙️ Server pushed data update (e.g., sensor value, HRM, TPMS, etc.)
//
//  5️⃣ DISCONNECTION PHASE
//     - esp_ble_gattc_close()  or  device out of range
//         ↓
//     → ESP_GATTC_DISCONNECT_EVT
//         ⚙️ BLE link terminated.
//         ⚠️ Cleanup connection data, stop timers, reset UI, etc.
//
// ----------------------------------------------------------------------------
// 💡 NOTE:
// - ESP_GATTC_CONNECT_EVT  → connection established (from controller)
// - ESP_GATTC_OPEN_EVT     → connection confirmed (from API)
// - Both may appear in quick succession — handle both safely.
//
// - Always check status codes (param->*.status) before acting.
// - Some events (READ, WRITE, NOTIFY) may repeat frequently during operation.
//
// ============================================================================
static void esp_gattc_callback(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if_param, esp_ble_gattc_cb_param_t *param)
{
    switch (event)
    {
        // ESP_GATTC_REG_EVT:
        // 🔹 Triggered after you call esp_ble_gattc_app_register()
        // 🔹 Means: "Your GATT client app has been registered with the BLE stack"
        //
        // ⚠️ IMPORTANT: Save the provided gattc_if handle!
        //     └─ You'll need it for all future GATT operations (connect, read, write, etc.)
        //
        // 🔹 This is usually the **first** event you receive after BLE initialization
        // 🔹 Once registration succeeds, you can start scanning or connecting to devices
        //
        // 💡 Tip: If status == ESP_GATT_OK → registration succeeded.
        //         Otherwise, check the error code for the reason of failure.
    case ESP_GATTC_REG_EVT:
        if (param->reg.status == ESP_GATT_OK)
        {
            ESP_LOGI(TAG_BLE_GATTC, LOG_GATTC_COLOR "GATT client registered, gattc_if = %d" LOG_ANSI_COLOR_RESET, gattc_if_param);
            gattc_if = gattc_if_param;
        }
        else
        {
            ESP_LOGE(TAG_BLE_GATTC, LOG_GATTC_COLOR "GATT client registration failed: %d", param->reg.status);
        }
        break;

        // ESP_GATTC_CONNECT_EVT:
        // 🔹 Triggered when the client successfully connects to a BLE device
        // 🔹 Means: "Connection established with the server"
        // ⚠️ IMPORTANT: Save the connection ID (conn_id) here!
        //     └─ This ID is required for all future GATT operations
        // 🔹 You can now start discovering services using:
        //     esp_ble_gattc_search_service(gattc_if, conn_id, NULL);
        // 🔹 param->connect.remote_bda gives the server’s Bluetooth address
    case ESP_GATTC_CONNECT_EVT:
    {
        auto connect = &param->connect;
        ESP_LOGI(TAG_BLE_GATTC, LOG_GATTC_COLOR "ESP_GATTC_CONNECT_EVT: " ESP_BD_ADDR_STR LOG_ANSI_COLOR_RESET, ESP_BD_ADDR_HEX(connect->remote_bda));
        break;
    }

        // ESP_GATTC_DISCONNECT_EVT:
        // 🔹 Triggered when the client disconnects from a BLE device
        // 🔹 Can happen either:
        //     - because you called esp_ble_gattc_close(), OR
        //     - because the remote device disconnected / went out of range
        //
        // ⚠️ IMPORTANT: Cleanup connection data here!
        //     └─ Clear or mark conn_id, remove device state from your connection list
        //     └─ Stop any timers, UI updates, or data subscriptions linked to this device
        //
        // 🔹 param->disconnect.reason gives the disconnection reason (BT status code)
        // 🔹 param->disconnect.remote_bda is the address of the disconnected device
        //
        // 💡 Tip: If you expect reconnection, you can restart scanning here.
    case ESP_GATTC_DISCONNECT_EVT:
    {
        auto disconnect = &param->disconnect;
        auto ad = disconnect->remote_bda;
        ESP_LOGI(TAG_BLE_GATTC, LOG_GATTC_COLOR "ESP_GATTC_DISCONNECT_EVT: " ESP_BD_ADDR_STR " conn_id:%d reason:%d" LOG_ANSI_COLOR_RESET, ESP_BD_ADDR_HEX(ad), disconnect->conn_id, disconnect->reason);
        break;
    }

        // ESP_GATTC_OPEN_EVT:
        // 🔹 Triggered after calling esp_ble_gattc_open() or esp_ble_gattc_aux_open()
        // 🔹 Means: "The connection attempt has completed"
        //
        // ⚠️ IMPORTANT: Check param->open.status!
        //     └─ ESP_GATT_OK → connection established successfully
        //     └─ Any other value → connection failed (device unreachable, timeout, etc.)
        //
        // ⚙️ If connection succeeded:
        //     └─ Save param->open.conn_id (connection ID)
        //     └─ You can now start discovering services via esp_ble_gattc_search_service()
        //
        // 🔹 param->open.remote_bda contains the address of the connected device
        // 🔹 param->open.mtu gives the negotiated MTU (if already exchanged)
        //
        // 💡 Tip: This event is the *confirmation* of a connection,
        //         while ESP_GATTC_CONNECT_EVT is the *notification* that it started.
    case ESP_GATTC_OPEN_EVT:
    {
        if (param->open.status == ESP_GATT_OK)
        {
            auto conn_id = param->open.conn_id;
            ESP_LOGI(TAG_BLE_GATTC, LOG_GATTC_COLOR "ESP_GATTC_OPEN_EVT Connected to COOSPO H6, conn_id = %d" LOG_ANSI_COLOR_RESET, conn_id);
            esp_ble_gattc_search_service(gattc_if, conn_id, NULL); // Поиск всех сервисов
        }
        else
        {
            ESP_LOGE(TAG_BLE_GATTC, LOG_GATTC_COLOR "ESP_GATTC_OPEN_EVT Connection failed, status = %d" LOG_ANSI_COLOR_RESET, param->open.status);
        }
        break;
    }

        // ESP_GATTC_READ_CHAR_EVT:
        // 🔹 Triggered after calling esp_ble_gattc_read_char() or esp_ble_gattc_read_by_handle()
        // 🔹 Means: "The GATT server has responded with the value of the characteristic"
        //
        // ⚙️ Happens during the DATA EXCHANGE phase (after services and characteristics are discovered)
        //
        // ⚠️ IMPORTANT:
        //     └─ Check param->read.status == ESP_GATT_OK before using the data
        //     └─ The actual value is in param->read.value, with length param->read.value_len
        //     └─ Use param->read.handle to identify which characteristic this value came from
        //
        // 🔹 Typical use:
        //     - Read sensor values, device info, or config data stored in a characteristic
        //     - Often used once after connection or after a configuration change
        //
        // 💡 Tip:
        //     - For continuous updates, use notifications instead (ESP_GATTC_NOTIFY_EVT)
        //     - You can queue multiple read requests, but handle them sequentially
        //
        // Example flow:
        //     esp_ble_gattc_read_char(gattc_if, conn_id, handle, ESP_GATT_AUTH_REQ_NONE);
        //         ↓
        //     → ESP_GATTC_READ_CHAR_EVT (with data)
    case ESP_GATTC_READ_CHAR_EVT:
    {
        auto &read = param->read;
        if (read.status == ESP_GATT_OK)
        {
            uint8_t *value = read.value;
            size_t len = read.value_len;
            ESP_LOGI(TAG_BLE_GATTC, LOG_GATTC_COLOR "Read value from handle %d, len: %d" LOG_ANSI_COLOR_RESET, read.handle, len);
            ESP_LOG_BUFFER_HEX(TAG_BLE_GATTC, value, len);

            // Зона фантазий
            static uint16_t device_name_handle = 0; // Дескриптор Device Name (0x2A00)
            // Проверяем, является ли это Device Name (0x2A00)
            if (len > 0 && device_name_handle == 0)
            {
                // Предполагаем, что это первый читаемый handle
                device_name_handle = read.handle;
                std::string device_name(reinterpret_cast<const char *>(value), len);

                ESP_LOGI(TAG_BLE_GATTC, LOG_GATTC_COLOR "Device Name: %s" LOG_ANSI_COLOR_RESET, device_name.c_str());
            }
        }
        else
        {

            ESP_LOGI(TAG_BLE_GATTC, LOG_GATTC_COLOR "ESP_GATTC_READ_CHAR_EVT status %x" LOG_ANSI_COLOR_RESET, (int)read.status);
        }
        break;
    }

        // ESP_GATTC_SEARCH_RES_EVT:
        // 🔹 Triggered during service discovery after calling esp_ble_gattc_search_service()
        // 🔹 Means: "One service has been found on the connected BLE server"
        //
        // ⚙️ Happens once per discovered service — you'll get multiple of these if the device has several services.
        //
        // ⚠️ IMPORTANT:
        //     └─ Each event describes ONE service:
        //          • param->search_res.srvc_id.uuid — UUID of the service
        //          • param->search_res.start_handle / end_handle — service handle range
        //          • param->search_res.conn_id — connection this belongs to
        //     └─ Save this info (e.g., into your vector or map of discovered services)
        //
        // 🔹 Typical next step:
        //     - After collecting all services, wait for ESP_GATTC_SEARCH_CMPL_EVT
        //       → then start discovering characteristics (esp_ble_gattc_get_characteristic)
        //
        // 💡 Tip:
        //     - You can filter by UUID here if you only care about specific services (like 0x180F = Battery)
        //     - Use this event to log or debug available services
        //
        // Example flow:
        //     esp_ble_gattc_search_service(gattc_if, conn_id, NULL);
        //         ↓
        //     → ESP_GATTC_SEARCH_RES_EVT (called multiple times — once per service)
        //         ↓
        //     → ESP_GATTC_SEARCH_CMPL_EVT (when all services are reported)
    case ESP_GATTC_SEARCH_RES_EVT:
    {
        auto &res = param->search_res;
        GATTServiceInfo service_info = {
            .start_handle = res.start_handle,
            .end_handle = res.end_handle,
            .uuid = res.srvc_id.uuid};
        found_services.push_back(service_info);

        // auto search_res = &param->search_res;
        // ESP_LOGI(TAG_BLE_GATTC, LOG_GATTC_COLOR "Service found: UUID len: %d, Start Handle: %d, End Handle: %d" LOG_ANSI_COLOR_RESET, search_res->srvc_id.uuid.len, search_res->start_handle, search_res->end_handle);
        //
        // if (search_res->srvc_id.uuid.len == ESP_UUID_LEN_16)
        //{
        //    if (search_res->srvc_id.uuid.uuid.uuid16 == 0x180D)
        //    {
        //        ESP_LOGI(TAG_BLE_GATTC, LOG_GATTC_COLOR "Found Heart Rate Service (0x180D), handle range: %d-%d" LOG_ANSI_COLOR_RESET, search_res->start_handle, search_res->end_handle);
        //    }
        //    else if (search_res->srvc_id.uuid.uuid.uuid16 == 0x180F)
        //    {
        //        ESP_LOGI(TAG_BLE_GATTC, LOG_GATTC_COLOR "Found Battery Service (0x180F), handle range: %d-%d" LOG_ANSI_COLOR_RESET, search_res->start_handle, search_res->end_handle);
        //    }
        //}
        break;
    }

        // ESP_GATTC_SEARCH_CMPL_EVT:
        // 🔹 Triggered after esp_ble_gattc_search_service() has finished
        // 🔹 Means: "Service discovery completed for this connection"
        //
        // ⚙️ Happens once per connection, after all ESP_GATTC_SEARCH_RES_EVT events are sent.
        //
        // ⚠️ IMPORTANT:
        //     └─ Check param->search_cmpl.status == ESP_GATT_OK
        //     └─ If OK → all services were successfully discovered
        //     └─ You can now proceed to discover characteristics or descriptors
        //          • esp_ble_gattc_get_characteristic()
        //          • esp_ble_gattc_get_descr_by_char_handle()
        //     └─ Use saved start_handle / end_handle ranges from SEARCH_RES events
        //
        // 🔹 Typical use:
        //     - Initialize characteristic discovery
        //     - Enable notifications for interesting characteristics
        //     - Read initial characteristic values if needed
        //
        // 💡 Tip:
        //     - This is a good place to log all found services
        //     - If device doesn’t respond with any services, status might still be OK,
        //       but your list will be empty → handle this case gracefully
        //
        // Example flow:
        //     esp_ble_gattc_search_service(gattc_if, conn_id, NULL);
        //         ↓
        //     → ESP_GATTC_SEARCH_RES_EVT (called once per service)
        //         ↓
        //     → ESP_GATTC_SEARCH_CMPL_EVT (discovery complete)
    case ESP_GATTC_SEARCH_CMPL_EVT:
    {
        print_found_services();

        if (param->search_cmpl.status == ESP_GATT_OK)
        {
            ESP_LOGI(TAG_BLE_GATTC, LOG_GATTC_COLOR "Service discovery completed, conn_id = %d" LOG_ANSI_COLOR_RESET, param->search_cmpl.conn_id);
        }
        else
        {
            ESP_LOGE(TAG_BLE_GATTC, LOG_GATTC_COLOR "Service discovery failed: %d" LOG_ANSI_COLOR_RESET, param->search_cmpl.status);
        }

        // Теперь можно читать характеристики (esp_ble_gattc_read_char)
        // Или подписаться на уведомления (esp_ble_gattc_register_for_notify).

        uint16_t count = 0;
        uint16_t offset = 0;
        esp_gatt_status_t ret_status = esp_ble_gattc_get_attr_count(gattc_if,
                                                                    conn_id,
                                                                    ESP_GATT_DB_CHARACTERISTIC, // ESP_GATT_DB_ALL ,,,
                                                                    1,
                                                                    65535,
                                                                    INVALID_HANDLE,
                                                                    &count);
        if (ret_status != ESP_GATT_OK)
        {
            ESP_LOGE(TAG_BLE_GATTC, "esp_ble_gattc_get_attr_count error, %d", __LINE__);
            break;
        }

        if (count > 0)
        {
            // static esp_gattc_char_elem_t *char_elem_result   = NULL;
            esp_gattc_char_elem_t *char_elem_result = (esp_gattc_char_elem_t *)malloc(sizeof(esp_gattc_char_elem_t) * count);
            if (!char_elem_result)
            {
                ESP_LOGE(TAG_BLE_GATTC, "gattc no mem");
                break;
            }
            else
            {
                memset(char_elem_result, 0xff, sizeof(esp_gattc_char_elem_t) * count);
                ret_status = esp_ble_gattc_get_all_char(gattc_if,
                                                        conn_id,
                                                        1,
                                                        65535,
                                                        char_elem_result,
                                                        &count,
                                                        offset);
                if (ret_status != ESP_GATT_OK)
                {
                    ESP_LOGE(TAG_BLE_GATTC, "esp_ble_gattc_get_all_char error, %d", __LINE__);
                    free(char_elem_result);
                    char_elem_result = NULL;
                    break;
                }
                if (count > 0)
                {

                    for (int i = 0; i < count; i++)
                    {
                        esp_gattc_char_elem_t &val = char_elem_result[i];
                        std::string info;

                        info.append(std::format("handle: {} ", val.char_handle));
                        info.append(format_uuid(val.uuid));
                        info.append(" Ability to:");

                        if (val.properties & ESP_GATT_CHAR_PROP_BIT_BROADCAST)
                            info.append("broadcast; ");
                        if (val.properties & ESP_GATT_CHAR_PROP_BIT_READ)
                            info.append("read; ");
                        if (val.properties & ESP_GATT_CHAR_PROP_BIT_WRITE_NR)
                            info.append("write without response; ");
                        if (val.properties & ESP_GATT_CHAR_PROP_BIT_WRITE)
                            info.append("write; ");
                        if (val.properties & ESP_GATT_CHAR_PROP_BIT_NOTIFY)
                            info.append("notify; ");
                        if (val.properties & ESP_GATT_CHAR_PROP_BIT_INDICATE)
                            info.append("indicate; ");
                        if (val.properties & ESP_GATT_CHAR_PROP_BIT_AUTH)
                            info.append("authenticate; ");
                        if (val.properties & ESP_GATT_CHAR_PROP_BIT_EXT_PROP)
                            info.append("Has extended properties;");

                        ESP_LOGI(TAG_BLE_GATTC, LOG_GATTC_COLOR "%s" LOG_ANSI_COLOR_RESET, info.c_str());

                        esp_ble_gattc_read_char(gattc_if, conn_id, val.char_handle, ESP_GATT_AUTH_REQ_NONE);

                        // if (char_elem_result[i].uuid.len == ESP_UUID_LEN_128)
                        //{
                        //     if (char_elem_result[i].properties & ESP_GATT_CHAR_PROP_BIT_NOTIFY && memcmp(char_elem_result[i].uuid.uuid.uuid128, notification_source, 16) == 0)
                        //     {
                        //         gl_profile_tab[PROFILE_A_APP_ID].notification_source_handle = char_elem_result[i].char_handle;
                        //         esp_ble_gattc_register_for_notify(gattc_if,
                        //                                           gl_profile_tab[PROFILE_A_APP_ID].remote_bda,
                        //                                           char_elem_result[i].char_handle);
                        //         ESP_LOGI(TAG_BLE_GATTC, "Find Apple noticification source char");
                        //     }
                        //     else if (char_elem_result[i].properties & ESP_GATT_CHAR_PROP_BIT_NOTIFY && memcmp(char_elem_result[i].uuid.uuid.uuid128, data_source, 16) == 0)
                        //     {
                        //         gl_profile_tab[PROFILE_A_APP_ID].data_source_handle = char_elem_result[i].char_handle;
                        //         esp_ble_gattc_register_for_notify(gattc_if,
                        //                                           gl_profile_tab[PROFILE_A_APP_ID].remote_bda,
                        //                                           char_elem_result[i].char_handle);
                        //         ESP_LOGI(TAG_BLE_GATTC, "Find Apple data source char");
                        //     }
                        //     else if (char_elem_result[i].properties & ESP_GATT_CHAR_PROP_BIT_WRITE && memcmp(char_elem_result[i].uuid.uuid.uuid128, control_point, 16) == 0)
                        //     {
                        //         gl_profile_tab[PROFILE_A_APP_ID].contol_point_handle = char_elem_result[i].char_handle;
                        //         ESP_LOGI(TAG_BLE_GATTC, "Find Apple control point char");
                        //     }
                        // }
                    }
                }
            }
            free(char_elem_result);
            char_elem_result = NULL;
        }

        break;
    }

        // ESP_GATTC_DIS_SRVC_CMPL_EVT:
        // 🔹 Triggered after calling esp_ble_gattc_close() or esp_ble_gattc_disconnect()
        // 🔹 Means: "Service discovery data and GATT database have been released for this connection"
        //
        // ⚙️ Happens when the BLE stack finishes cleaning up all internal GATT structures
        //    associated with a connection that has been closed.
        //
        // ⚠️ IMPORTANT:
        //     └─ This is a *post-disconnection cleanup* event.
        //     └─ All handles, service lists, and cached attributes for this conn_id are now invalid.
        //     └─ Do NOT access any service or characteristic data from this connection after this point.
        //
        // 🔹 Typical use:
        //     - Finalize disconnection sequence
        //     - Free any user-side memory or objects linked to this device
        //     - Prepare for a possible reconnection
        //
        // 💡 Tip:
        //     - Often follows ESP_GATTC_DISCONNECT_EVT automatically
        //     - If you reconnect to the same device, the GATT DB will be rediscovered
        //
        // Example flow:
        //     esp_ble_gattc_close(gattc_if, conn_id);
        //         ↓
        //     → ESP_GATTC_DISCONNECT_EVT  (link lost or closed)
        //         ↓
        //     → ESP_GATTC_DIS_SRVC_CMPL_EVT (stack finished cleanup)
    case ESP_GATTC_DIS_SRVC_CMPL_EVT: /*!< When the ble discover service complete, the event comes */
    {
        if (param->search_cmpl.status == ESP_GATT_OK)
        {
            ESP_LOGI(TAG_BLE_GATTC, LOG_GATTC_COLOR "BLE discover service complete, conn_id = %d" LOG_ANSI_COLOR_RESET, param->search_cmpl.conn_id);
        }
        else
        {
            ESP_LOGE(TAG_BLE_GATTC, LOG_GATTC_COLOR "BLE discover service failed: %d" LOG_ANSI_COLOR_RESET, param->search_cmpl.status);
        }
        break;
    }

    // ESP_GATTC_NOTIFY_EVT:
// 🔹 Triggered when the server sends a value update (notification or indication)
// 🔹 Means: "The characteristic value was pushed from the server to the client"
//
// ⚠️ IMPORTANT:
//     └─ param->notify.is_notify == true  → Notification (no ack required)
//     └─ param->notify.is_notify == false → Indication (ack handled by stack)
//     └─ Use param->notify.handle to know WHICH characteristic sent the data
//     └─ The bytes are in param->notify.value (length = param->notify.value_len)
//
// 🔹 Typical use:
//     - Continuous sensor updates (heart rate, TPMS, etc.)
//     - Real-time streams without polling (more efficient than read)
//
// 💡 To receive this event you MUST:
//     1) Register for notifications:
//          esp_ble_gattc_register_for_notify(gattc_if, remote_bda, char_handle);
//     2) Enable the CCCD (0x2902) descriptor of that characteristic:
//          write 0x0001 → notifications ON
//          write 0x0002 → indications  ON
//          (0x0003 → both; 0x0000 → off)
//
// Example flow:
//     discover services/characteristics → get char_handle & cccd_handle
//         ↓
//     esp_ble_gattc_register_for_notify(gattc_if, remote_bda, char_handle);
//         ↓
//     esp_ble_gattc_write_char_descr(gattc_if, conn_id, cccd_handle,
//                                    sizeof(uint16_t), (uint8_t*)"\x01\x00",
//                                    ESP_GATT_WRITE_TYPE_RSP, ESP_GATT_AUTH_REQ_NONE);
//         ↓
//     → ESP_GATTC_NOTIFY_EVT (called whenever the server pushes an update)
    case ESP_GATTC_NOTIFY_EVT: /*!< When the ble discover service complete, the event comes */
    {
        if (param->search_cmpl.status == ESP_GATT_OK)
        {
            ESP_LOGI(TAG_BLE_GATTC, LOG_GATTC_COLOR "BLE discover service complete, conn_id = %d" LOG_ANSI_COLOR_RESET, param->search_cmpl.conn_id);
        }
        else
        {
            ESP_LOGE(TAG_BLE_GATTC, LOG_GATTC_COLOR "BLE discover service failed: %d" LOG_ANSI_COLOR_RESET, param->search_cmpl.status);
        }
        break;
    }

    default:
        ESP_LOGE(TAG_BLE_GATTC, LOG_GATTC_COLOR "esp_gattc_callback !!! event: %d" LOG_ANSI_COLOR_RESET, (int)event);
        break;
    }
}

#pragma endregion

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
