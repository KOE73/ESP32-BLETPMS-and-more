

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
#include <optional>

#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_gap_ble_api.h"

#include "yabt_events.hpp"
#include "yabt_TomTom_TPMS.hpp"

namespace yabt
{
    // Определение статического экземпляра (создаётся автоматически)
    BtDeviceRecognizerTomTomTPMS BtDeviceRecognizerTomTomTPMS::instance;

    /*
    bytes 0 and 1 -->0001 Manufacturer (see https://www.bluetooth.com/specifications/assigned-numbers/company-identifiers/)
    byte 2 -->80 Sensor Number (80:1, 81:2, 82:3, 83:4, ...)
    bytes 3 and 4 -->EACA Address Prefix
    bytes 5, 6 and 7--> 108A78 Sensor Address --> usually listed on the TPMS box
    bytes 8, 9, 10 and 11 -->E36D0000 Tire pressure (in kPA)
    bytes 12, 13, 14 and 15 -->E60A0000 Tire Temperature (in Celsius)
    byte 16 -->5B Battery Percentage
    byte 17 -->00 Alarm Flag (00: Ok, 01: No pressure)

    "000180eaca108a78e36d0000e60a00005b00"
     0001                                    Manufacturer (0001: TomTom)
         80                                  Sensor Number (80:1, 81:2, 82:3, 83:4, ..)
         80eaca108a78                        Sensor Address
                     e36d0000                Pressure
                             e60a0000        Temperature
                                     5b      Battery percentage
                                       00    Alarm Flag (00: OK, 01: No Pressure Alarm)

    BT Addr 82:EA:CA:30:07:27
    02 01 05 03 03 b0 fb 13 ff 00 01 82 ea ca 30 07 27 5b 86 03 00 3f 09 00 00 42 00
    02 01 05
             03 03 b0 fb
                         13 ff 00 01 82 ea ca 30 07 27 5b 86 03 00 3f 09 00 00 42 00
                               00 01 82 ea ca 30 07 27 5b 86 03 00 3f 09 00 00 42 00

    Flags, Complete List of 16-bit Service or Service Class UUIDs, Manufacturer Specific Data
    Flags:                                                    5 Limited discoverable, No BR/EDR support
    16BitServiceUUIDs ESP_BLE_AD_TYPE_16SRV_CMPL = 0x03   =>  0xFBB0
    */

    bool BtDeviceRecognizerTomTomTPMS::GapHandler(const BleGapExtAdvReport &report)
    {
        auto oRaw = report.getManufacturerData();
        if (!oRaw.has_value())
            return false;

        auto raw = oRaw.value();

        if (raw.size() != 18)
            return false;

        if (raw[0] != 0x00 || raw[1] != 0x01)
            return false;

        if (raw[3] != 0xEA || raw[4] != 0xCA)
            return false;

        return true;
    }

    TPMSData BtDeviceRecognizerTomTomTPMS::parseTPMSData(const BleGapExtAdvReport &report)
    {
        auto rawData = report.getManufacturerData().value();
        TPMSData data;
        strncpy(data.id, report.getAddr().toString().c_str(), TPMSDATA_ID_LENGTH);

        if (rawData.size() != 18)
        {
            return data; // throw std::invalid_argument("Invalid data size");
        }
        
        strncpy(data.manufacturerName, "TomTom", TPMSDATA_MANUFACTERENAME_LENGTH);

        data.sensorNumber = rawData[2] - 0x80;
        data.sensorAddress = rawData[5] | (rawData[6] << 8) | (rawData[7] << 16);
        data.batteryPercentage = rawData[16];
        data.alarmFlag = rawData[17];

        uint32_t tirePressurePa = rawData[8] | (rawData[9] << 8) | (rawData[10] << 16) | (rawData[11] << 24);
        data.pressureRaw = tirePressurePa;
        // Преобразование в промежуточные единицы
        data.pressure_kPa = static_cast<float>(tirePressurePa) * PA_TO_KPA;   // Pa -> kPa
        data.pressure_mbar = static_cast<float>(tirePressurePa) * PA_TO_MBAR; // Pa -> mbar

        // Преобразование в конечные единицы
        data.pressure_Psi = data.pressure_kPa * KPA_TO_PSI;
        data.pressure_Bar = data.pressure_mbar * MBAR_TO_BAR;
        data.pressure_KgCm2 = data.pressure_kPa * KPA_TO_KG_PER_CM2;
        data.pressure_Atm = data.pressure_kPa * KPA_TO_ATM;

        uint32_t tireTemperature = rawData[12] | (rawData[13] << 8) | (rawData[14] << 16) | (rawData[15] << 24);
        data.temperatureRaw = tireTemperature;
        data.temperatureC = tireTemperature / 100.0;
        data.temperatureF = (data.temperatureC * CELSIUS_TO_FAHRENHEIT_FACTOR) + FAHRENHEIT_OFFSET;

        return data;
    }

    void BtDeviceRecognizerTomTomTPMS::Log(const BleGapExtAdvReport &report)
    {
        ESP_LOGI(TAG_BTController, " **************************** Device recognized by: %s", getName());

        auto a = report.getManufacturerData();
        auto s = parseTPMSData(a.value());

        ESP_LOGI(TAG_BTController, "Sensor Number: 0x%02X", s.sensorNumber);
        ESP_LOGI(TAG_BTController, "Sensor Address: 0x%06" PRIX32, s.sensorAddress);
        ESP_LOGI(TAG_BTController, "Tire Pressure: 0x%08" PRIX32, s.pressureRaw);
        ESP_LOGI(TAG_BTController, "Tire Temperature: 0x%08" PRIX32, s.temperatureRaw);
        ESP_LOGI(TAG_BTController, "Tire Pressure: %.5f kPa", s.pressure_kPa);
        ESP_LOGI(TAG_BTController, "Tire Pressure: %.5f mbar", s.pressure_mbar);
        ESP_LOGI(TAG_BTController, "Tire Pressure: %.5f Psi", s.pressure_Psi);
        ESP_LOGI(TAG_BTController, "Tire Pressure: %.5f Bar", s.pressure_Bar);
        ESP_LOGI(TAG_BTController, "Tire Pressure: %.5f KgCm2", s.pressure_KgCm2);
        ESP_LOGI(TAG_BTController, "Tire Pressure: %.5f Atm", s.pressure_Atm);

        ESP_LOGI(TAG_BTController, "Tire Temperature: %.5f °C", s.temperatureC);
        ESP_LOGI(TAG_BTController, "Tire Temperature: %.5f °F", s.temperatureF);

        ESP_LOGI(TAG_BTController, "Battery Percentage: 0x%02X %%", s.batteryPercentage);
        ESP_LOGI(TAG_BTController, "Battery Percentage: %d %%", s.batteryPercentage);
        ESP_LOGI(TAG_BTController, "Alarm Flag: 0x%02X", s.alarmFlag);
    }

    void BtDeviceRecognizerTomTomTPMS::SendEvent(esp_event_loop_handle_t yabt_loop, const BleGapExtAdvReport &report)
    {
        // ESP_LOGI("TAG_BTController", "PreSend YABT_EVENT_TPMS");

        auto data = report.getManufacturerData();
        auto tpmsData = parseTPMSData(data.value());

        // Отправка
        ESP_ERROR_CHECK_WITHOUT_ABORT(esp_event_post_to(
            yabt_loop,
            YABT_EVENT,         // База событий
            YABT_EVENT_TPMS,    // ID события
            &tpmsData,          // Указатель на данные (опционально)
            sizeof(TPMSData),   // Размер данных
            pdMS_TO_TICKS(3000) // ms or portMAX_DELAY     // Тайм-аут (ожидание, если очередь полна)
            ));

        // ESP_LOGI("TAG_BTController", "PostSend YABT_EVENT_TPMS");
    }

} // namespace yabt
