#pragma once

#include <cstdint>
#include "yabt.hpp"

namespace yabt
{
    // Базовые единицы и их порядок (float)
    constexpr float PA_TO_KPA = 0.001f; // 1 Pa = 0.001 kPa
    constexpr float PA_TO_MBAR = 0.01f; // 1 Pa = 0.01 mbar

    // Коэффициенты перевода из базовых единиц (float)
    constexpr float KPA_TO_PSI = 0.145037738f;     // 1 kPa = 0.145037738 psi
    constexpr float MBAR_TO_BAR = 0.001f;          // 1 mbar = 0.001 bar
    constexpr float KPA_TO_KG_PER_CM2 = 0.010197f; // 1 kPa = 0.010197 kg/cm²
    constexpr float KPA_TO_ATM = 0.00986923f;      // 1 kPa = 0.00986923 atm

    constexpr float CELSIUS_TO_FAHRENHEIT_FACTOR = 9.0f / 5.0f; // 1.8
    constexpr float FAHRENHEIT_OFFSET = 32.0f;

#define TPMSDATA_MANUFACTERENAME_LENGTH 50

    struct TPMSData : public DeviceData
    {
        char manufacturerName[TPMSDATA_MANUFACTERENAME_LENGTH];
        uint8_t sensorNumber;
        uint32_t sensorAddress;
        uint32_t pressureRaw;
        uint32_t temperatureRaw;
        uint8_t batteryPercentage;
        uint8_t alarmFlag;

        float pressure_kPa;
        float pressure_mbar;
        float pressure_Psi;
        float pressure_Bar;
        float pressure_KgCm2;
        float pressure_Atm;

        float temperatureC;
        float temperatureF;
    };

} // namespace yabt
