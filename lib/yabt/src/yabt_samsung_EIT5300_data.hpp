#pragma once

#include <cstdint>
#include "yabt.hpp"

namespace yabt
{
    /*
        Samsung SmartTag EI-T5300 — BLE Advertisement (Service Data UUID 0xFD5A)
        ----------------------------------------------------------
        Источник: community reverse (uTag/wiki), SmartThings Find protocol research.
        Рекламные данные несут базовую информацию об устройстве
        без спаривания: состояние, батарея, флаги, регион, privacy-ID, подпись.

        Пример Service Data (UUID 0xFD5A):
            5A FD 10 43 15 00 04 42 7F 9A 0E 62 20 07 10 01
              |  |  |  |  |  |  |  |  |  |  |  |  |  |  |__ signature/reserved
              |  |  |  |  |  |  |  |  |  |  |  |  |  |____ byte15  - signature/reserved
              |  |  |  |  |  |  |  |  |  |  |  |  |_______ bytes12-14 – Privacy ID (rotating)
              |  |  |  |  |  |  |  |  |  |  |_____________ bytes7-8   – Region / variant
              |  |  |  |  |  |  |  |  |____________________ byte6     – Flags (UWB/E2E/Motion)
              |  |  |  |  |  |_____________________________ byte5     – Battery level (0-3)
              |  |  |  |__________________________________ byte4     – Ageing counter (ticks)
              |  |  |_____________________________________ byte3     – Tag State (0x01-0x06)
              |  |________________________________________ byte2     – Protocol version (0x10)
              |___________________________________________ bytes0-1  – Service UUID (FD5A)

        Поля (известное значение):
            Tag State:
                0x01 Premature Offline
                0x02 Offline
                0x03 Overmature Offline
                0x04 Connected (Normal)
                0x05 / 0x06 Connected w/ motion or alert
            Battery Level:
                0x00 Very Low
                0x01 Low
                0x02 Medium
                0x03 Full
            Flags (битовое поле):
                bit0 – E2E encryption enabled
                bit1 – UWB hardware present
                bit2 – Lost mode
                bit3 – Motion detected
                другие биты зарезервированы
            Privacy ID:
                3-байтовое значение, ротируется каждые ~15 мин
            Ageing Counter:
                счётчик, увеличивается со временем, используется для проверки свежести рекламы

        Используется для:
            - пассивного обнаружения SmartTag'ов в сети SmartThings Find
            - парсинга состояния устройства без BLE-подключения
    */

    struct SmartTagFD5AData : public DeviceData
    {
        static constexpr const char *DEFAULT_TYPE = "SmartTag";

        uint8_t protocolVersion; // byte2
        uint8_t tagState;        // byte3
        uint8_t ageingCounter;   // byte4
        uint8_t batteryLevel;    // byte5
        uint8_t flags;           // byte6
        uint16_t region;         // bytes7-8
        uint32_t privacyId;      // bytes9-11 (3 байта, упакованы в младшие)
        uint32_t signature;      // bytes12-15 (резерв/подпись, 4 байта)

        // Доп. вычисленные поля (по желанию)
        bool motionDetected;
        bool uwbPresent;
        bool e2eEnabled;
        bool lostMode;

        SmartTagFD5AData() : DeviceData(DEFAULT_TYPE)
        {
            protocolVersion = 0;
            tagState = 0;
            ageingCounter = 0;
            batteryLevel = 0;
            flags = 0;
            region = 0;
            privacyId = 0;
            signature = 0;

            motionDetected = false;
            uwbPresent = false;
            e2eEnabled = false;
            lostMode = false;
        }

        // Простая интерпретация флагов
        void parseFlags()
        {
            e2eEnabled = flags & 0x01;
            uwbPresent = flags & 0x02;
            lostMode = flags & 0x04;
            motionDetected = flags & 0x08;
        }

        // Распечатка для отладки (по желанию)
        void debugPrint() const
        {
            printf("SmartTag FD5A: ver=%02X state=%02X batt=%u region=%04X priv=%06lX flags=%02X\n",
                   protocolVersion, tagState, batteryLevel, region, privacyId, flags);
        }
    };

} // namespace yabt
