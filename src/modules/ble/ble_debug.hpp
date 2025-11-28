#pragma once

#include <string>

//#include "esp_bt.h"
//#include "esp_bt_main.h"
#include "esp_gap_ble_api.h"

#include "yabt_utils.hpp"


/**
 * @brief Processes an extended advertising report.
 *
 * This function takes an extended advertising report (`esp_ble_gap_ext_adv_report_t`),
 * formats its fields into human-readable strings, and logs them for debugging purposes.
 *
 * The extracted details include:
 * 
 * - Event type
 * 
 * - Address and address type
 * 
 * - Primary and secondary PHY types
 * 
 * - RSSI (signal strength)
 * 
 * - Advertising data and its length
 *
 * @param report The extended advertising report to process.
 */
void process_ext_adv_report(const yabt::BleGapExtAdvReport &report);

/**
 * @brief Handles BLE GAP events and logs advertising data.
 *
 * This callback function processes BLE GAP events, specifically extended
 * advertising reports. It extracts advertising data fields and logs them
 * for debugging purposes.
 *
 * The function iterates through all possible advertising data types, extracts
 * the relevant data, and formats it into human-readable strings.
 *
 * @param report The extended advertising report containing the data to process.
 */
void esp_gap_cb(esp_ble_gap_ext_adv_report_t &report);

std::string const process_adv_data(const uint8_t *data, uint8_t data_len, esp_ble_adv_data_type type);



