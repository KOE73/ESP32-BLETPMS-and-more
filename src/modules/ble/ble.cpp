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
                                                     /* ELM327 need BLE_SCAN_TYPE_ACTIVE ? */
    .own_addr_type = BLE_ADDR_TYPE_PUBLIC,           /* BLE_ADDR_TYPE_PUBLIC BLE_ADDR_TYPE_RANDOM BLE_ADDR_TYPE_RPA_PUBLIC BLE_ADDR_TYPE_RPA_RANDOM */
    .scan_filter_policy = BLE_SCAN_FILTER_ALLOW_ALL, /* BLE_SCAN_FILTER_ALLOW_ALL BLE_SCAN_FILTER_ALLOW_ONLY_WLST BLE_SCAN_FILTER_ALLOW_UND_RPA_DIR BLE_SCAN_FILTER_ALLOW_WLIST_RPA_DIR */
    .scan_interval = 0x80,                          /*  */
    .scan_window = 0x40,                             /* duty cycle < 100% for WiFI work (0x100, 0xE0) */
    .scan_duplicate = BLE_SCAN_DUPLICATE_DISABLE     /* BLE_SCAN_DUPLICATE_DISABLE BLE_SCAN_DUPLICATE_ENABLE (BLE5)BLE_SCAN_DUPLICATE_ENABLE_RESET*/
};

esp_ble_ext_scan_params_t ext_scan_params = {
    .own_addr_type = BLE_ADDR_TYPE_PUBLIC,                                                 /* BLE_ADDR_TYPE_PUBLIC BLE_ADDR_TYPE_RANDOM BLE_ADDR_TYPE_RPA_PUBLIC BLE_ADDR_TYPE_RPA_RANDOM */
    .filter_policy = BLE_SCAN_FILTER_ALLOW_ALL,                                            /* BLE_SCAN_FILTER_ALLOW_ALL BLE_SCAN_FILTER_ALLOW_ONLY_WLST BLE_SCAN_FILTER_ALLOW_UND_RPA_DIR BLE_SCAN_FILTER_ALLOW_WLIST_RPA_DIR */
    .scan_duplicate = BLE_SCAN_DUPLICATE_DISABLE,                                          /* BLE_SCAN_DUPLICATE_DISABLE BLE_SCAN_DUPLICATE_ENABLE (BLE5)BLE_SCAN_DUPLICATE_ENABLE_RESET*/
    .cfg_mask = ESP_BLE_GAP_EXT_SCAN_CFG_UNCODE_MASK /*| ESP_BLE_GAP_EXT_SCAN_CFG_CODE_MASK*/, /* Scan Advertisements on the LE1M PHY | on the LE coded PHY */
    .uncoded_cfg = {BLE_SCAN_TYPE_PASSIVE, 0x80, 0x70},                                   /* duty cycle < 100% for WiFI work (0x100, 0xE0)*/
    .coded_cfg = {BLE_SCAN_TYPE_PASSIVE, 0x80, 0x70},                                     /* duty cycle < 100% for WiFI work (0x100, 0xE0)*/
                                                                                           /* ELM327 need BLE_SCAN_TYPE_ACTIVE ? */
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

#pragma region -GAP

#pragma region ble_gap_callback_legacy

// ============================================================================
// 🧭 GAP EVENT FLOW — BLE 4.2 Scanning & Discovery Lifecycle
// ----------------------------------------------------------------------------
// This callback (ble_gap_callback_legacy) handles all GAP events for
// **classic BLE scanning** (non-extended). It covers the full device
// discovery flow — from setting scan parameters to detecting nearby devices.
//
// 🔄 Typical Flow:
//  1️⃣ SET SCAN PARAMETERS
//     - esp_ble_gap_set_scan_params(&scan_params);
//         ↓
//     → ESP_GAP_BLE_SCAN_PARAM_SET_COMPLETE_EVT
//         ⚙️ Indicates the controller accepted scan configuration
//         ✅ Next step → start scanning via esp_ble_gap_start_scanning(duration)
//
//  2️⃣ START SCANNING
//     - esp_ble_gap_start_scanning(duration_seconds);
//         ↓
//     → ESP_GAP_BLE_SCAN_START_COMPLETE_EVT
//         ⚙️ Confirms scanning has actually started
//         ⚠️ If failed → check controller state or scan params
//
//  3️⃣ RECEIVE ADVERTISING PACKETS
//     - While scanning is active, multiple:
//         → ESP_GAP_BLE_SCAN_RESULT_EVT
//             ⚙️ Each event = one advertising or scan-response packet
//             💡 Parse param->scan_rst to identify nearby devices
//             🔍 Common fields:
//                 • bda          → device MAC address
//                 • rssi         → signal strength
//                 • ble_adv[]    → raw advertising data
//             ✅ Use this stage to filter devices by name, manufacturer, etc.
//
//  4️⃣ SCAN COMPLETED
//     - Happens automatically (after timeout) or manually:
//         esp_ble_gap_stop_scanning();
//             ↓
//         → ESP_GAP_BLE_SCAN_STOP_COMPLETE_EVT
//             ⚙️ Confirms scanning stopped successfully
//             💡 Typical next step → initiate connection to chosen device
//
// ----------------------------------------------------------------------------
// ⚙️ Optional / Related Events:
// - ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT → when this device sets up advertising
// - ESP_GAP_BLE_AUTH_CMPL_EVT / KEY_EVT   → appear if pairing or bonding enabled
// - ESP_GAP_BLE_UPDATE_CONN_PARAMS_EVT    → after connection, link parameters change
//
// 💡 NOTES:
// - This callback only covers *GAP discovery* (advertising & scanning).
// - All *GATT connection* events (connect, read, write, notify) are handled
//   separately in `esp_gattc_callback`.
// - In BLE 5.0, extended scanning uses ble_gap_callback_ext() with
//   ESP_GAP_BLE_EXT_* events (same logic, richer data).
//
// ============================================================================
static bool ble_gap_callback_legacy(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)
{
    switch (event)
    {
        // ESP_GAP_BLE_SCAN_PARAM_SET_COMPLETE_EVT:
        // 🔹 Triggered after you call esp_ble_gap_set_scan_params() or esp_ble_gap_set_ext_scan_params()
        // 🔹 Means: "The BLE scan parameters have been successfully configured"
        //
        // ⚙️ Happens once the controller accepts your scan parameters (interval, window, policy, etc.)
        //
        // ⚠️ IMPORTANT:
        //     └─ Check param->scan_param_cmpl.status == ESP_BT_STATUS_SUCCESS
        //     └─ If success → you can now start scanning using:
        //            esp_ble_gap_start_scanning(duration_seconds);
        //        or for BLE 5.0 extended scanning:
        //            esp_ble_gap_start_ext_scan(duration_ms, period_ms);
        //
        // 🔹 Typical next step:
        //     - Start scanning for advertising devices
        //     - Begin discovering nearby peripherals
        //
        // 🔹 param->scan_param_cmpl.status gives the operation result
        //
        // 💡 Tip:
        //     - This event only signals *configuration complete*, not that scanning has started.
        //     - If you see this event but scanning doesn’t start, check your `esp_ble_gap_start_*` call timing.
        //     - Common cause of failure: BLE controller not initialized or wrong address type.
        //
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

        // ESP_GAP_BLE_SCAN_START_COMPLETE_EVT:
        // 🔹 Triggered right after calling esp_ble_gap_start_scanning()
        // 🔹 Means: "The BLE scan process has been started (or failed to start)"
        //
        // ⚙️ Happens once the controller acknowledges your request to begin scanning.
        //
        // ⚠️ IMPORTANT:
        //     └─ Check param->scan_start_cmpl.status == ESP_BT_STATUS_SUCCESS
        //     └─ If success → device is now actively scanning for advertisements
        //     └─ If failed  → check BLE state, previous scan still running, or invalid params
        //
        // 🔹 Typical next step:
        //     - Wait for ESP_GAP_BLE_SCAN_RESULT_EVT events (each representing an advertising packet)
        //     - Handle discovered devices, filter by RSSI, manufacturer data, etc.
        //
        // 🔹 Common causes of failure:
        //     - Scanning already in progress
        //     - Controller busy with another GAP operation
        //     - Invalid scan parameters not yet applied
        //
        // 💡 Tip:
        //     - You don’t need to manually restart scanning inside this event — just confirm success.
        //     - To stop scanning later: esp_ble_gap_stop_scanning();
        //     - Combine with ESP_GAP_BLE_SCAN_STOP_COMPLETE_EVT for a clean start/stop flow.
        //
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

        // ESP_GAP_BLE_SCAN_RESULT_EVT:
        // 🔹 Triggered whenever a BLE advertising packet or scan response is received
        // 🔹 Means: "A nearby BLE device was detected during active or passive scanning"
        //
        // ⚙️ This event fires *repeatedly* — once per advertising report
        //     Each report describes one packet (ADV_IND, ADV_SCAN_RSP, etc.)
        //
        // ⚠️ IMPORTANT:
        //     └─ The specific result type is in param->scan_rst.search_evt
        //     └─ Use a switch on search_evt to handle different sub-events:
        //
        //        • ESP_GAP_SEARCH_INQ_RES_EVT → Device found (main advertising report)
        //        • ESP_GAP_SEARCH_INQ_CMPL_EVT → Inquiry complete (scan finished)
        //        • ESP_GAP_SEARCH_DISC_RES_EVT → Discovery result
        //        • ESP_GAP_SEARCH_DISC_CMPL_EVT → Discovery complete
        //        • ESP_GAP_SEARCH_SEARCH_CANCEL_CMPL_EVT → Search cancelled
        //
        // 🔹 Typical use:
        //     - Parse param->scan_rst to extract data fields:
        //         • param->scan_rst.bda → Device MAC address
        //         • param->scan_rst.rssi → Signal strength (dBm)
        //         • param->scan_rst.ble_adv / adv_data_len → Raw advertising payload
        //
        // 🔹 You can decode advertising data (AD structures) to find:
        //     - Local name
        //     - Manufacturer data
        //     - Service UUIDs
        //     - Flags, etc.
        //
        // 💡 Tip:
        //     - This is the main event for device discovery and filtering logic
        //     - Store devices in a list or map, keyed by address
        //     - When a desired device is found, stop scanning and initiate connection
        //
        // Example flow:
        //     esp_ble_gap_start_scanning(duration);
        //         ↓
        //     → ESP_GAP_BLE_SCAN_RESULT_EVT (fires multiple times)
        //         ↓
        //     → ESP_GAP_BLE_SCAN_STOP_COMPLETE_EVT (when duration ends or manually stopped)
        //
    case ESP_GAP_BLE_SCAN_RESULT_EVT:
        switch (param->scan_rst.search_evt)
        {
            ////
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

        // ESP_GAP_BLE_SCAN_STOP_COMPLETE_EVT:
        // 🔹 Triggered after calling esp_ble_gap_stop_scanning()
        // 🔹 Means: "The BLE scanning process has been stopped (successfully or with an error)"
        //
        // ⚙️ Happens once the controller halts ongoing scanning and frees radio resources.
        //
        // ⚠️ IMPORTANT:
        //     └─ Check param->scan_stop_cmpl.status == ESP_BT_STATUS_SUCCESS
        //     └─ If success → no more ESP_GAP_BLE_SCAN_RESULT_EVT will be triggered
        //     └─ If failed  → check if scanning was already stopped or BLE was reset
        //
        // 🔹 Typical next step:
        //     - Optionally initiate a connection to a discovered device using:
        //           esp_ble_gattc_open(gattc_if, remote_bda, addr_type, direct);
        //     - Or restart scanning if you are running periodic discovery.
        //
        // 🔹 Common causes of failure:
        //     - Scanning was already stopped or timed out automatically
        //     - Controller busy or not in scanning state
        //
        // 💡 Tip:
        //     - Always stop scanning before starting a GATT connection.
        //     - Use this event as a clean transition point between "scan → connect" phases.
        //     - Extended scanning (BLE 5.0) has its own stop event: ESP_GAP_BLE_EXT_SCAN_STOP_COMPLETE_EVT.
        //
        // Example flow:
        //     esp_ble_gap_start_scanning(duration);
        //         ↓
        //     → ESP_GAP_BLE_SCAN_RESULT_EVT (multiple times)
        //         ↓
        //     esp_ble_gap_stop_scanning();
        //         ↓
        //     → ESP_GAP_BLE_SCAN_STOP_COMPLETE_EVT
        //
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

    // ============================================================================
    // 🧭 OTHER ESP_GAP_BLE EVENTS — Not Yet Implemented
    // ----------------------------------------------------------------------------
    // ✅ Recommended to Implement (useful in most real BLE apps)
    // ----------------------------------------------------------------------------
    // ESP_GAP_BLE_UPDATE_CONN_PARAMS_EVT:
    // 🔹 Triggered when connection parameters (interval, latency, timeout) are updated
    // 🔹 Useful to monitor link quality or optimize battery/performance tradeoffs
    //
    // ESP_GAP_BLE_AUTH_CMPL_EVT:
    // 🔹 Triggered when BLE pairing/authentication completes
    // 🔹 Provides peer device info and result of encryption/bonding
    //
    // ESP_GAP_BLE_KEY_EVT:
    // 🔹 Key exchange event during pairing (LTK, IRK, CSRK, etc.)
    // 🔹 Needed if you enable security or bonding
    //
    // ESP_GAP_BLE_PASSKEY_REQ_EVT / ESP_GAP_BLE_PASSKEY_NOTIF_EVT:
    // 🔹 Request/display PIN for secure pairing
    // 🔹 Needed if using ESP_LE_AUTH_REQ_SC_BOND or similar security modes
    //
    // ESP_GAP_BLE_REMOVE_BOND_DEV_COMPLETE_EVT:
    // 🔹 Confirms that a bonded device has been removed from storage
    // 🔹 Useful for managing bond list in secure applications
    //
    // ESP_GAP_BLE_SET_LOCAL_PRIVACY_COMPLETE_EVT:
    // 🔹 Called when random/private address setup completes
    // 🔹 Required if using BLE privacy (RPA addresses)
    // ----------------------------------------------------------------------------
    // ⚙️ Optional / Rarely Needed (advanced or niche use)
    // ----------------------------------------------------------------------------
    // ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT:
    // 🔹 Advertising data configured (for peripheral or beacon mode)
    //
    // ESP_GAP_BLE_SCAN_RSP_DATA_SET_COMPLETE_EVT:
    // 🔹 Scan response data configured (for peripheral mode)
    //
    // ESP_GAP_BLE_ADV_START_COMPLETE_EVT:
    // 🔹 Advertising started (for peripheral/beacon role)
    //
    // ESP_GAP_BLE_ADV_STOP_COMPLETE_EVT:
    // 🔹 Advertising stopped
    //
    // ESP_GAP_BLE_SET_STATIC_RAND_ADDR_EVT:
    // 🔹 Random static address was successfully set
    //
    // ESP_GAP_BLE_SET_EXT_ADV_PARAMS_COMPLETE_EVT:
    // 🔹 Extended advertising parameters configured (BLE 5.0 peripheral)
    //
    // ESP_GAP_BLE_EXT_ADV_START_COMPLETE_EVT:
    // 🔹 Extended advertising started (BLE 5.0 peripheral)
    //
    // ESP_GAP_BLE_EXT_ADV_STOP_COMPLETE_EVT:
    // 🔹 Extended advertising stopped
    //
    // ESP_GAP_BLE_PERIODIC_ADV_REPORT_EVT:
    // 🔹 Received periodic advertisement (BLE 5.0 feature)
    //
    // ESP_GAP_BLE_PERIODIC_ADV_SYNC_ESTAB_EVT / SYNC_LOST_EVT:
    // 🔹 Periodic advertising sync established/lost (advanced BLE 5)
    //
    // ----------------------------------------------------------------------------
    // 💡 Tip:
    // You can safely ignore most "ADV_*" events unless your ESP32 acts as a
    // peripheral, beacon, or broadcaster. For scanning/monitoring applications,
    // the main flow is already covered by your implemented events.
    //
    // ============================================================================
}

#pragma endregion ble_gap_callback_legacy

#pragma region ble_gap_callback_ext

// ============================================================================
// 🧭 GAP EVENT FLOW — BLE 5.0 Extended Scanning Lifecycle
// ----------------------------------------------------------------------------
// This callback (ble_gap_callback_ext) handles all GAP events for
// **BLE 5.0 extended scanning and advertising**. It supports scanning
// across multiple PHYs (1M / Coded) and richer advertising reports.
//
// 🔄 Typical Flow (BLE 5.0):
//  1️⃣ SET EXTENDED SCAN PARAMETERS
//     - esp_ble_gap_set_ext_scan_params(&ext_scan_params);
//         ↓
//     → ESP_GAP_BLE_SET_EXT_SCAN_PARAMS_COMPLETE_EVT
//         ⚙️ Confirms the controller accepted extended scan configuration.
//         ✅ Next step → start scanning via:
//              esp_ble_gap_start_ext_scan(duration_ms, period_ms);
//
//  2️⃣ START EXTENDED SCANNING
//     - esp_ble_gap_start_ext_scan(30000, 1000); // e.g. 30s duration, 1s period
//         ↓
//     → ESP_GAP_BLE_EXT_SCAN_START_COMPLETE_EVT
//         ⚙️ Confirms scanning started on configured PHYs (1M, Coded, etc.)
//         ⚠️ If failed → check controller BLE 5 support and config mask.
//
//  3️⃣ RECEIVE EXTENDED ADVERTISING REPORTS
//         → ESP_GAP_BLE_EXT_ADV_REPORT_EVT
//             ⚙️ Fired for *each* extended advertising packet detected.
//             💡 Each param->ext_adv_report.params contains:
//                 • addr             → advertiser address
//                 • adv_data[]       → full advertising payload
//                 • adv_data_len     → data length
//                 • primary_phy / secondary_phy → PHY used
//                 • rssi             → signal strength (dBm)
//             ✅ Parse the report to detect known devices (via manufacturer ID,
//                UUIDs, names, etc.).
//
//             🔸Typical logic:
//                 - Decode advertising structures (AD types)
//                 - Identify known sensors (TPMS, HRM, etc.)
//                 - Log or trigger recognition events
//
//  4️⃣ STOP EXTENDED SCANNING
//     - Manually or automatically after duration:
//           esp_ble_gap_stop_ext_scan();
//         ↓
//     → ESP_GAP_BLE_EXT_SCAN_STOP_COMPLETE_EVT
//         ⚙️ Confirms scanning stopped successfully
//         💡 Typical next step → connect to the found device
//
//  5️⃣ (Optional) PERIODIC ADVERTISING
//         → ESP_GAP_BLE_PERIODIC_ADV_REPORT_EVT
//             ⚙️ Report from advertisers that use periodic packets (BLE 5 feature).
//             💡 Can be used for low-power telemetry sensors or beacons.
//
// ----------------------------------------------------------------------------
// ⚙️ Optional / Related Extended GAP Events:
// - ESP_GAP_BLE_SET_EXT_ADV_PARAMS_COMPLETE_EVT → when configuring extended advertising
// - ESP_GAP_BLE_EXT_ADV_START_COMPLETE_EVT / STOP_COMPLETE_EVT → when this device advertises
// - ESP_GAP_BLE_PERIODIC_ADV_SYNC_ESTAB_EVT / SYNC_LOST_EVT → periodic adv sync control
//
// 💡 NOTES:
// - Extended scanning allows simultaneous reception on 1M and Coded PHYs.
// - Advertising data may exceed the 31-byte limit of BLE 4.2 packets.
// - The logic mirrors legacy scanning, but uses richer data structures.
// - All *connection* logic still goes through `esp_gattc_callback`.
//
// ============================================================================
static bool ble_gap_callback_ext(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)
{
    switch (event)
    {
    // ESP_GAP_BLE_SET_EXT_SCAN_PARAMS_COMPLETE_EVT:
    // 🔹 Triggered after calling esp_ble_gap_set_ext_scan_params()
    // 🔹 Means: "The controller has successfully configured extended scan parameters"
    //
    // ⚙️ This event confirms that BLE 5.0 scanning parameters (PHYs, interval, window,
    //    filter policy, duplicate filtering, etc.) were accepted by the controller.
    //
    // ⚠️ IMPORTANT:
    //     └─ Check param->set_ext_scan_params.status == ESP_BT_STATUS_SUCCESS
    //     └─ If success → you can safely start scanning via:
    //            esp_ble_gap_start_ext_scan(duration_ms, period_ms);
    //     └─ If failed  → check your ext_scan_params values or BLE 5.0 support in firmware
    //
    // 🔹 Typical next step:
    //     - Start scanning for extended advertising reports:
    //            esp_ble_gap_start_ext_scan(30000, 1000);
    //     - Or run indefinitely (duration = 0) for continuous discovery.
    //
    // 🔹 Common causes of failure:
    //     - Invalid combination of PHYs or cfg_mask
    //     - BLE controller not initialized or busy
    //     - BLE 4.2-only chip/firmware (no BLE5 support)
    //
    // 💡 Tip:
    //     - Extended scanning enables simultaneous reception on 1M + Coded PHYs.
    //     - Use `ESP_BLE_GAP_EXT_SCAN_CFG_UNCODE_MASK` / `CODE_MASK` to control PHYs.
    //     - This event only means parameters are accepted — scanning itself begins
    //       after the next event ESP_GAP_BLE_EXT_SCAN_START_COMPLETE_EVT.
    //
    // Example flow:
    //     esp_ble_gap_set_ext_scan_params(&ext_scan_params);
    //         ↓
    //     → ESP_GAP_BLE_SET_EXT_SCAN_PARAMS_COMPLETE_EVT
    //         ↓
    //     esp_ble_gap_start_ext_scan(duration, period);
    //         ↓
    //     → ESP_GAP_BLE_EXT_SCAN_START_COMPLETE_EVT
    //
    case ESP_GAP_BLE_SET_EXT_SCAN_PARAMS_COMPLETE_EVT:
        if (param->set_ext_scan_params.status == ESP_BT_STATUS_SUCCESS)
        {
            ESP_LOGI(TAG_BLE_CALLBACK, "Extended scan params set, starting scan...");
            //esp_ble_gap_start_ext_scan(40, 60);
            esp_ble_gap_start_ext_scan(0, 0); // Бесконечное сканирование
        }
        return true;

    // ESP_GAP_BLE_EXT_SCAN_START_COMPLETE_EVT:
    // 🔹 Triggered after calling esp_ble_gap_start_ext_scan()
    // 🔹 Means: "Extended BLE scanning has successfully started (or failed to start)"
    //
    // ⚙️ Happens once the BLE controller begins active or passive scanning on the
    //    configured PHYs (1M and/or Coded). This event confirms that the request
    //    from esp_ble_gap_start_ext_scan() was accepted and activated.
    //
    // ⚠️ IMPORTANT:
    //     └─ Check param->ext_scan_start.status == ESP_BT_STATUS_SUCCESS
    //     └─ If success → device is now actively scanning for extended advertisements
    //     └─ If failed  → verify BLE 5.0 support, PHY config mask, or controller state
    //
    // 🔹 Typical next step:
    //     - Wait for multiple ESP_GAP_BLE_EXT_ADV_REPORT_EVT events
    //       → each represents one extended advertising packet
    //     - Parse advertising data, detect known devices, etc.
    //
    // 🔹 Common causes of failure:
    //     - Another scan already running
    //     - BLE controller busy or disabled
    //     - Invalid scan configuration (e.g. both PHYs disabled)
    //
    // 💡 Tip:
    //     - For long-running scans, use duration = 0 (infinite scanning).
    //     - You can stop scanning anytime via esp_ble_gap_stop_ext_scan().
    //     - This event marks the **transition from setup → active scan phase**.
    //
    // Example flow:
    //     esp_ble_gap_set_ext_scan_params(&ext_scan_params);
    //         ↓
    //     → ESP_GAP_BLE_SET_EXT_SCAN_PARAMS_COMPLETE_EVT
    //         ↓
    //     esp_ble_gap_start_ext_scan(30000, 1000);
    //         ↓
    //     → ESP_GAP_BLE_EXT_SCAN_START_COMPLETE_EVT  ✅ (now scanning)
    //         ↓
    //     → ESP_GAP_BLE_EXT_ADV_REPORT_EVT ×N
    //         ↓
    //     esp_ble_gap_stop_ext_scan();
    //         ↓
    //     → ESP_GAP_BLE_EXT_SCAN_STOP_COMPLETE_EVT
    //
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

        // ESP_GAP_BLE_EXT_ADV_REPORT_EVT:
        // 🔹 Triggered whenever an extended BLE advertising packet is received
        // 🔹 Means: "A nearby BLE 5.0 device has broadcasted an extended advertisement"
        //
        // ⚙️ This is the **core event** of BLE 5.0 scanning — fires continuously
        //    for every advertising report detected on 1M and/or Coded PHYs.
        //
        // ⚠️ IMPORTANT:
        //     └─ Each call contains one advertising report in:
        //            param->ext_adv_report.params
        //     └─ You can safely parse, filter, or process data here
        //        (do NOT block for long — runs in BLE callback context)
        //
        // 🔹 param->ext_adv_report.params fields:
        //     • addr             → advertiser address (esp_ble_addr_t)
        //     • addr_type        → public / random / RPA
        //     • primary_phy      → PHY where adv started (1M or Coded)
        //     • secondary_phy    → PHY of secondary channel (if any)
        //     • adv_data         → raw advertising payload
        //     • adv_data_len     → length in bytes
        //     • rssi             → signal strength in dBm
        //     • event_type       → ADV_IND, ADV_EXT_IND, SCAN_RSP, etc.
        //
        // 🔹 Typical use:
        //     - Parse the advertising payload (AD structures)
        //     - Detect known devices by manufacturer ID, UUIDs, or local name
        //     - Log or dispatch events to higher-level recognizers
        //
        // Example parsing logic:
        //     const auto &report = param->ext_adv_report.params;
        //     yabt::BleGapExtAdvReport adv(report);
        //     BTController::getInstance().GapHanler(adv);
        //
        // 🔹 Common subfields to extract:
        //     - Complete Local Name (AD type 0x09)
        //     - Manufacturer Data (0xFF) → report.getManufacturerData()
        //     - Service UUIDs (16/32/128-bit)
        //     - Flags, Tx Power, Appearance
        //
        // 💡 Tips:
        //     - This event may fire very frequently — handle efficiently.
        //     - For better performance, move parsing into a lightweight object
        //       (as done via yabt::BleGapExtAdvReport).
        //     - If you detect your target → stop scan:
        //           esp_ble_gap_stop_ext_scan();
        //
        // Example flow:
        //     esp_ble_gap_start_ext_scan(...);
        //         ↓
        //     → ESP_GAP_BLE_EXT_SCAN_START_COMPLETE_EVT
        //         ↓
        //     → ESP_GAP_BLE_EXT_ADV_REPORT_EVT × N (device packets)
        //         ↓
        //     esp_ble_gap_stop_ext_scan();
        //         ↓
        //     → ESP_GAP_BLE_EXT_SCAN_STOP_COMPLETE_EVT
        //
    case ESP_GAP_BLE_EXT_ADV_REPORT_EVT:
    {
        // esp_ble_gap_ext_adv_report_t *report = &param->ext_adv_report.params;

        const yabt::BleGapExtAdvReport Report(param->ext_adv_report.params);
        if (yabt::BTController::getInstance().GapHanler(Report))
            return true;
        process_ext_adv_report(Report);
        return true;

        // Evry BLE SHOW---// ESP_LOGI(TAG_BLE_CALLBACK, " ~~~~ %s", Report.getAddr().toString().c_str());

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

        // ESP_GAP_BLE_EXT_SCAN_STOP_COMPLETE_EVT:
        // 🔹 Triggered after calling esp_ble_gap_stop_ext_scan()
        // 🔹 Means: "Extended BLE scanning has been successfully stopped (or failed)"
        //
        // ⚙️ Happens once the controller halts extended scanning on all PHYs (1M/Coded)
        //    and frees radio resources. This marks the end of the discovery phase.
        //
        // ⚠️ IMPORTANT:
        //     └─ Check param->scan_stop_cmpl.status == ESP_BT_STATUS_SUCCESS
        //     └─ If success → no more ESP_GAP_BLE_EXT_ADV_REPORT_EVT will be triggered
        //     └─ If failed  → scanning might have already stopped or BLE stack busy
        //
        // 🔹 Typical next step:
        //     - Connect to the selected device via:
        //           esp_ble_gattc_aux_open(gattc_if, remote_bda, BLE_ADDR_TYPE_RANDOM, true);
        //       or esp_ble_gattc_open() for legacy connections.
        //     - Or restart scanning if continuous discovery is desired.
        //
        // 🔹 Common causes of failure:
        //     - Scanning was already stopped automatically (timeout)
        //     - Controller busy or connection in progress
        //     - Invalid state transition (stop called twice)
        //
        // 💡 Tip:
        //     - Always stop scanning before opening a GATT connection.
        //     - For long-running apps, consider restarting scanning periodically
        //       to refresh device visibility.
        //     - This event is your clean “handover” point from SCAN → CONNECT phase.
        //
        // Example flow:
        //     esp_ble_gap_set_ext_scan_params(&ext_scan_params);
        //         ↓
        //     → ESP_GAP_BLE_SET_EXT_SCAN_PARAMS_COMPLETE_EVT
        //         ↓
        //     esp_ble_gap_start_ext_scan(30000, 1000);
        //         ↓
        //     → ESP_GAP_BLE_EXT_SCAN_START_COMPLETE_EVT
        //         ↓
        //     → ESP_GAP_BLE_EXT_ADV_REPORT_EVT × N
        //         ↓
        //     esp_ble_gap_stop_ext_scan();
        //         ↓
        //     → ESP_GAP_BLE_EXT_SCAN_STOP_COMPLETE_EVT  ✅ (safe to connect)
        //
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

        // ESP_GAP_BLE_PERIODIC_ADV_REPORT_EVT:
        // 🔹 Triggered when a periodic advertising packet is received from a BLE 5.0 device
        // 🔹 Means: "A device is broadcasting synchronized periodic advertisements"
        //
        // ⚙️ This event occurs after your scanner has synchronized to a periodic advertiser
        //    (typically using periodic advertising sync). The packets arrive at fixed intervals
        //    and can contain sensor or beacon data with low power overhead.
        //
        // ⚠️ IMPORTANT:
        //     └─ The report data is stored in param->period_adv_report.params
        //     └─ Fields include:
        //          • sync_handle → unique ID for the periodic sync
        //          • rssi         → signal strength in dBm
        //          • data         → raw payload data
        //          • data_length  → length of data
        //     └─ You can parse this payload the same way as extended advertisements.
        //
        // 🔹 Typical use:
        //     - Continuous telemetry (TPMS, heart rate belts, environment sensors)
        //     - Beacons with fixed-interval broadcast data
        //     - Ultra-low-power data streams (sensors that sleep between broadcasts)
        //
        // 🔹 Example handling:
        //     esp_ble_gap_periodic_adv_report_t *report = &param->period_adv_report.params;
        //     ESP_LOGI(TAG_BLE_CALLBACK,
        //              "Periodic Adv Report - SyncHandle:%d  RSSI:%d",
        //              report->sync_handle, report->rssi);
        //     ESP_LOG_BUFFER_HEX(TAG_BLE_CALLBACK, report->data, report->data_length);
        //
        // 💡 Tips:
        //     - Periodic advertising requires prior synchronization (SYNC_ESTAB event).
        //     - Use esp_ble_gap_periodic_adv_sync_start() to subscribe to a known advertiser.
        //     - Once synchronized, reports arrive automatically until stopped or lost.
        //     - To end reception, call esp_ble_gap_periodic_adv_sync_terminate(sync_handle).
        //
        // Example flow:
        //     esp_ble_gap_start_ext_scan(...);
        //         ↓
        //     → ESP_GAP_BLE_EXT_ADV_REPORT_EVT (detect advertiser with periodic support)
        //         ↓
        //     esp_ble_gap_periodic_adv_sync_start(...);
        //         ↓
        //     → ESP_GAP_BLE_PERIODIC_ADV_SYNC_ESTAB_EVT (sync established)
        //         ↓
        //     → ESP_GAP_BLE_PERIODIC_ADV_REPORT_EVT × N (periodic packets)
        //         ↓
        //     → ESP_GAP_BLE_PERIODIC_ADV_SYNC_LOST_EVT (sync lost or terminated)
        //
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

    // ============================================================================
    // 🧭 OTHER ESP_GAP_BLE EVENTS — Not Yet Handled in ble_gap_callback_ext()
    // ----------------------------------------------------------------------------
    // ✅ Recommended to Implement (useful or likely to appear)
    // ----------------------------------------------------------------------------
    // case ESP_GAP_BLE_UPDATE_CONN_PARAMS_EVT:
    //     // 🔹 Triggered when connection parameters (interval, latency, timeout) change
    //     // 🔹 Useful to monitor and log link stability or optimize performance
    //     // ⚙️ Example:
    //     //     auto &p = param->update_conn_params;
    //     //     ESP_LOGI(TAG_BLE_CALLBACK, "Conn params updated: interval=%d latency=%d timeout=%d",
    //     //              p.int_min, p.latency, p.timeout);
    //     break;
    //
    // case ESP_GAP_BLE_AUTH_CMPL_EVT:
    //     // 🔹 Triggered when authentication (pairing/bonding) completes
    //     // 🔹 Provides security level, device address, and auth result
    //     // ⚙️ Use this to confirm successful pairing
    //     // 💡 Required if you connect to secured BLE devices
    //     break;
    //
    // case ESP_GAP_BLE_KEY_EVT:
    //     // 🔹 Reports key exchange during pairing (LTK, IRK, CSRK, etc.)
    //     // 🔹 Rare unless bonding enabled
    //     // ⚙️ Example: store keys for future reconnection
    //     break;
    //
    // case ESP_GAP_BLE_PASSKEY_REQ_EVT:
    // case ESP_GAP_BLE_PASSKEY_NOTIF_EVT:
    //     // 🔹 Used for PIN-based pairing (display or input passkey)
    //     // ⚙️ Needed only if you enable numeric comparison / passkey authentication
    //     break;
    //
    // case ESP_GAP_BLE_REMOVE_BOND_DEV_COMPLETE_EVT:
    //     // 🔹 Confirms bonded device removal
    //     // ⚙️ Helpful if you manage bonding lists dynamically
    //     break;
    //
    // case ESP_GAP_BLE_SET_LOCAL_PRIVACY_COMPLETE_EVT:
    //     // 🔹 Triggered after enabling privacy (RPA addresses)
    //     // ⚙️ Use when working with random/private address rotation
    //     break;
    //
    // ----------------------------------------------------------------------------
    // ⚙️ Optional / Rarely Needed (mostly for peripheral or advertising mode)
    // ----------------------------------------------------------------------------
    // case ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT:
    //     // 🔹 Advertising data configured for this ESP32 (peripheral/beacon mode)
    //     break;
    //
    // case ESP_GAP_BLE_SCAN_RSP_DATA_SET_COMPLETE_EVT:
    //     // 🔹 Scan-response payload configured (peripheral mode)
    //     break;
    //
    // case ESP_GAP_BLE_ADV_START_COMPLETE_EVT:
    //     // 🔹 Advertising started successfully
    //     break;
    //
    // case ESP_GAP_BLE_ADV_STOP_COMPLETE_EVT:
    //     // 🔹 Advertising stopped
    //     break;
    //
    // case ESP_GAP_BLE_SET_STATIC_RAND_ADDR_EVT:
    //     // 🔹 Random static address successfully assigned
    //     break;
    //
    // case ESP_GAP_BLE_SET_EXT_ADV_PARAMS_COMPLETE_EVT:
    //     // 🔹 Extended advertising parameters configured (BLE 5.0 peripheral mode)
    //     break;
    //
    // case ESP_GAP_BLE_EXT_ADV_START_COMPLETE_EVT:
    // case ESP_GAP_BLE_EXT_ADV_STOP_COMPLETE_EVT:
    //     // 🔹 Extended advertising started/stopped (BLE 5.0 peripheral mode)
    //     break;
    //
    // case ESP_GAP_BLE_PERIODIC_ADV_SYNC_ESTAB_EVT:
    // case ESP_GAP_BLE_PERIODIC_ADV_SYNC_LOST_EVT:
    //     // 🔹 Periodic advertising sync established or lost (BLE 5.0 feature)
    //     // ⚙️ Appears when syncing to periodic advertisers
    //     break;
    //
    // ----------------------------------------------------------------------------
    // 💡 Tip:
    // These events are safe to ignore unless your ESP32 acts as a *peripheral*
    // or requires *bonding / secure pairing*. For scanning and recognition use-cases,
    // your existing handled events (SET_EXT_SCAN_PARAMS → EXT_SCAN_START → EXT_ADV_REPORT → EXT_SCAN_STOP)
    // already cover the full BLE 5.0 discovery cycle.
    //
    // ============================================================================
}

#pragma endregion ble_gap_callback_ext

static void ble_gap_callback(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)
{
    if (ble_gap_callback_ext(event, param))
        return;

    if (ble_gap_callback_legacy(event, param))
        return;

    ESP_LOGW(TAG_BLE_CALLBACK, "Unhandled GAP event: %d", (int)event);
}

#pragma endregion GAP

#pragma region -GATCC

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

#pragma region esp_gattc_callback

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

#pragma endregion esp_gattc_callback

#pragma endregion GATCC

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

    ESP_LOGI(TAG_BLE, "BLE initialized.");
}

#pragma region TEST HR

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
#pragma endregion