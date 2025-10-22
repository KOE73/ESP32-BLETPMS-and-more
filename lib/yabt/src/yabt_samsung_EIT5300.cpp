#include "string.h"

#include <vector>
#include <memory>
#include <unordered_map>
#include <string>
#include <array>
#include <iostream>
#include <iomanip>
#include <map>
#include <optional>
#include <span>

#include "esp_bt.h"
#include "esp_event.h"
#include "esp_bt_main.h"
#include "esp_gap_ble_api.h"

#include "yabt.hpp"
#include "yabt_samsung_EIT5300.hpp"

#include <nlohmann/json.hpp>

#define TAG "SMARTTAG"

namespace yabt
{
    // Статический экземпляр, создаётся автоматически при старте
    BtDeviceRecognizerSmartTag BtDeviceRecognizerSmartTag::instance;

    /*
        Samsung SmartTag EI-T5300 — BLE Advertising (Service Data UUID 0xFD5A)
        ----------------------------------------------------------------------
        Пример пакета:
            02 01 06 03 03 5A FD 17 16 5A FD 10 43 15 00 04 42 7F 9A 0E 62 20 07 10 01

        bytes 0-1   -> 5A FD   Service UUID (SmartTag)
        byte  2     -> 10      Protocol Version
        byte  3     -> 43      Tag State (01-Premature, 02-Offline, 03-Overmature, 04+-Connected)
        byte  4     -> 15      Ageing Counter
        byte  5     -> 00      Battery Level (00-VeryLow, 01-Low, 02-Med, 03-Full)
        byte  6     -> 04      Flags (bit0-E2E, bit1-UWB, bit2-Lost, bit3-Motion)
        bytes 7-8   -> 42 7F   Region
        bytes 9-11  -> 9A 0E 62 Privacy ID (rotating ~15 min)
        bytes 12-15 -> 20 07 10 01 Signature/Reserved

        "5AFD1043150004427F9A0E6220071001"
    */

    bool BtDeviceRecognizerSmartTag::CanHandle(const BleGapExtAdvReport &report)
    {
        auto payload = report.getServiceDataPayload(0xFD5A);
        return payload && payload->size() >= 14; // типичная длина
    }

    bool BtDeviceRecognizerSmartTag::parseSmartTagData(const BleGapExtAdvReport &report,
                                                       SmartTagFD5AData &data)
    {
        auto rawOpt = report.getServiceDataPayload(0xFD5A);
        if (!rawOpt.has_value())
            return false;

        auto raw = rawOpt.value();
        // Payload (после UUID) — ожидаем минимум ~14 байт
        if (raw.size() < 14)
            return false;

        // id (не забудь обеспечить '\0')
        std::string addrStr = report.getAddr().toString();
        std::strncpy(data.id, addrStr.c_str(), DEVICEDATA_ID_LENGTH);
        if (DEVICEDATA_ID_LENGTH > 0)
            data.id[DEVICEDATA_ID_LENGTH - 1] = '\0';

        // ---- Смещения уже ПОСЛЕ UUID ----
        // raw[0] = protocolVersion
        // raw[1] = tagState
        // raw[2] = ageingCounter
        // raw[3] = batteryLevel
        // raw[4] = flags
        // raw[5..6] = region (LE)
        // raw[7..9] = privacyId (3B, LE)
        // raw[10..13] = signature (4B, LE)

        data.protocolVersion = raw[0];
        data.tagState = raw[1];
        data.ageingCounter = raw[2];
        data.batteryLevel = raw[3];
        data.flags = raw[4];
        data.region = static_cast<uint16_t>(raw[5] | (raw[6] << 8));

        data.privacyId = static_cast<uint32_t>(raw[7] | (raw[8] << 8) | (raw[9] << 16));

        data.signature = static_cast<uint32_t>(raw[10] |
                                               (raw[11] << 8) |
                                               (raw[12] << 16) |
                                               (raw[13] << 24));

        data.parseFlags(); // выставить e2eEnabled/uwbPresent/lostMode/motionDetected

        return true;
    }

    void BtDeviceRecognizerSmartTag::Log(const BleGapExtAdvReport &report)
    {
        ESP_LOGI(TAG, "**************************** Device recognized by: %s", getName());

        SmartTagFD5AData st;
        if (!parseSmartTagData(report, st))
        {
            ESP_LOGW(TAG, "SmartTag FD5A payload missing or truncated");
            return;
        }

        ESP_LOGI(TAG, "Protocol Ver: 0x%02X", st.protocolVersion);
        ESP_LOGI(TAG, "Tag State   : 0x%02X", st.tagState);
        ESP_LOGI(TAG, "Age Counter : %u", st.ageingCounter);
        ESP_LOGI(TAG, "Battery Lv  : %u", st.batteryLevel);
        ESP_LOGI(TAG, "Flags       : 0x%02X", st.flags);
        ESP_LOGI(TAG, "Region      : 0x%04X", st.region);
        ESP_LOGI(TAG, "Privacy ID  : 0x%06" PRIX32, st.privacyId); // 3-байтовое значение
        ESP_LOGI(TAG, "Signature   : 0x%08" PRIX32, st.signature);
        ESP_LOGI(TAG, "E2E:%d UWB:%d Lost:%d Motion:%d",
                 st.e2eEnabled, st.uwbPresent, st.lostMode, st.motionDetected);
    }

    void BtDeviceRecognizerSmartTag::SendEvent(esp_event_loop_handle_t yabt_loop,
                                               const BleGapExtAdvReport &report)
    {
        SmartTagFD5AData st;
        parseSmartTagData(report, st);

        nlohmann::json j;
        j["device_address"] = report.getAddr().toString();
        j["protocol_ver"] = st.protocolVersion;
        j["tag_state"] = st.tagState;
        j["age_counter"] = st.ageingCounter;
        j["battery_level"] = st.batteryLevel;
        j["flags"] = st.flags;
        j["region"] = st.region;
        j["privacy_id"] = st.privacyId;
        j["signature"] = st.signature;
        j["motion_detected"] = st.motionDetected;
        j["uwb_present"] = st.uwbPresent;
        j["e2e_enabled"] = st.e2eEnabled;
        j["lost_mode"] = st.lostMode;

        std::string jsonStr = j.dump();
        ESP_LOGI(TAG, "JSON Data: %s", jsonStr.c_str());

        ESP_ERROR_CHECK_WITHOUT_ABORT(esp_event_post_to(
            yabt_loop,
            YABT_EVENT,
            YABT_EVENT_SMARTTAG,
            &st,
            sizeof(SmartTagFD5AData),
            pdMS_TO_TICKS(3000)));
    }

} // namespace yabt
