#include <string>
#include <format>

#include <stdio.h>
#include <string.h>

#include "esp_system.h"
#include "esp_log.h"
#include "esp_event.h"
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_gap_ble_api.h"

#include "ble_debug.hpp"

// #include "esp_gattc_api.h"

// static const char *TAG_BLE = "BLE";
static const char *TAG_BLE_CALLBACK = "BLE_CALLBACK";

void process_ext_adv_report(const yabt::BleGapExtAdvReport &report)
{

    return;

    ESP_LOGI(TAG_BLE_CALLBACK, "───────────────────────────────────────────────");
    ESP_LOGI(TAG_BLE_CALLBACK, "📡 Extended Advertising Report");
    ESP_LOGI(TAG_BLE_CALLBACK, "───────────────────────────────────────────────");

    // === Базовая информация ===
    ESP_LOGI(TAG_BLE_CALLBACK, "Event Type:   %s", report.getEventTypeStr().c_str());
    ESP_LOGI(TAG_BLE_CALLBACK, "Address:      %s", report.getAddr().toString().c_str());
    ESP_LOGI(TAG_BLE_CALLBACK, "Addr Type:    %s", report.getAddrTypeStr().c_str());
    ESP_LOGI(TAG_BLE_CALLBACK, "Primary PHY:  %s", report.getPrimaryPhyStr().c_str());
    ESP_LOGI(TAG_BLE_CALLBACK, "Secondary PHY:%s", report.getSecondlyPhyStr().c_str());
    ESP_LOGI(TAG_BLE_CALLBACK, "RSSI:         %s dBm", report.getRssiStr().c_str());
    ESP_LOGI(TAG_BLE_CALLBACK, "Data Status:  %s", report.getDataStatusStr().c_str());
    ESP_LOGI(TAG_BLE_CALLBACK, "SID:          %s", report.getSidStr().c_str());
    ESP_LOGI(TAG_BLE_CALLBACK, "Tx Power:     %d dBm", report.getTxPowerStr().c_str());

    // === Дополнительные поля ===
    ESP_LOGI(TAG_BLE_CALLBACK, "Periodic Adv Interval: %s", report.getPerAdvIntervalStr().c_str());
    ESP_LOGI(TAG_BLE_CALLBACK, "Direct Address:        %s", report.getDirAddrStr().c_str());

    // === Имя устройства ===
    if (auto name = report.getCompleteLocalName())
        ESP_LOGI(TAG_BLE_CALLBACK, "Device Name:  %s", name.value().c_str());

    // === UUID’ы (16/32/128-bit) ===
    ESP_LOGI(TAG_BLE_CALLBACK, "16-bit UUIDs: %s", report.get16BitServiceUUIDsAsString().c_str());
    ESP_LOGI(TAG_BLE_CALLBACK, "32-bit UUIDs: %s", report.get32BitServiceUUIDsAsString().c_str());
    ESP_LOGI(TAG_BLE_CALLBACK, "128-bit UUIDs:%s", report.get128BitServiceUUIDsAsString().c_str());

    // === Производитель ===
    if (auto manufacturerId = report.getManufacturerId())
        ESP_LOGI(TAG_BLE_CALLBACK, "Manufacturer Id: 0x%04X", manufacturerId.value());

    if (auto manufacturerName = report.getManufacturerName())
        ESP_LOGI(TAG_BLE_CALLBACK, "Manufacturer Name: %s", manufacturerName.value().c_str());

    if (auto manufacturerData = report.getManufacturerDataAsString())
        ESP_LOGI(TAG_BLE_CALLBACK, "Manufacturer Data: %s", manufacturerData.value().c_str());

    if (auto flags = report.getActiveFlagsDescription())
    {
        ESP_LOGI(TAG_BLE_CALLBACK, "Flags:        0x%02X (%s)", flags.value().c_str());
    }

    ESP_LOGI(TAG_BLE_CALLBACK, "───────────────────────────────────────────────");
}

// This function processes an extended advertising report (esp_ble_gap_ext_adv_report_t)
// It formats the fields of the report into human-readable strings and logs them for debugging.
// The function extracts details such as event type, address, PHY types, RSSI, advertising data, etc.
// and outputs them using ESP_LOGI for easier analysis of BLE advertising packets.
void process_ext_adv_report11111(const esp_ble_gap_ext_adv_report_t &report)
{
    // Format each field of the extended advertising report into a human-readable string
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

    // Format advertising data as a hex string
    std::string adv_data_str = "Adv Data: ";
    for (int i = 0; i < report.adv_data_len; i++)
    {
        adv_data_str += std::format("{:02x} ", report.adv_data[i]);
    }

    // Log all formatted strings for debugging purposes
    ESP_LOGI(TAG_BLE_CALLBACK, "%s", event_type_str.c_str());
    ESP_LOGI(TAG_BLE_CALLBACK, "%s", addr_type_str.c_str());
    ESP_LOGI(TAG_BLE_CALLBACK, "%s", addr_str.c_str());
    ESP_LOGI(TAG_BLE_CALLBACK, "%s", primary_phy_str.c_str());
    ESP_LOGI(TAG_BLE_CALLBACK, "%s", secondary_phy_str.c_str());
    ESP_LOGI(TAG_BLE_CALLBACK, "%s", sid_str.c_str());
    // ESP_LOGI(TAG_BLE_CALLBACK, "%s", tx_power_str.c_str());
    ESP_LOGI(TAG_BLE_CALLBACK, "%s", rssi_str.c_str());
    // ESP_LOGI(TAG_BLE_CALLBACK, "%s", per_adv_interval_str.c_str());
    // ESP_LOGI(TAG_BLE_CALLBACK, "%s", dir_addr_type_str.c_str());
    // ESP_LOGI(TAG_BLE_CALLBACK, "%s", dir_addr_str.c_str());
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
std::string const process_adv_data(const uint8_t *data, uint8_t data_len, esp_ble_adv_data_type type)
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
        std::string result;
        result += "Target Address: ";
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
        std::string result;
        result += type == ESP_BLE_AD_TYPE_SPAIR_C256 ? "Secure Pairing C256: " : "Secure Pairing R256: ";
        for (int i = 0; i < length; i++)
        {
            result += std::format("{:02x} ", value[i]);
        }
        return result;
    }
    case ESP_BLE_AD_TYPE_LE_SECURE_CONFIRM:
    case ESP_BLE_AD_TYPE_LE_SECURE_RANDOM:
    {
        std::string result;
        result += type == ESP_BLE_AD_TYPE_LE_SECURE_CONFIRM ? "LE Secure Confirm: " : "LE Secure Random: ";
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
        std::string result;
        result += "Transport Discovery Data: ";
        for (int i = 0; i < length; i++)
        {
            result += std::format("{:02x} ", value[i]);
        }
        return result;
    }
    case ESP_BLE_AD_TYPE_LE_SUPPORT_FEATURE:
    {
        std::string result;
        result += "LE Supported Features: ";
        for (int i = 0; i < length; i++)
        {
            result += std::format("{:02x} ", value[i]);
        }
        return result;
    }
    case ESP_BLE_AD_TYPE_CHAN_MAP_UPDATE:
    {
        std::string result;
        result += "Channel Map Update: ";
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
