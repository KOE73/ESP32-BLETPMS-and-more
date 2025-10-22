#pragma once

#include "string.h"

// #include <iostream>
#include <vector>
#include <memory>
#include <unordered_map>
#include <string>
#include <cstring>
#include <array>
#include <iostream>
#include <iomanip>
#include <map>
#include <optional>
#include <span>
#include <sstream>

// #include "esp_system.h"
// #include "esp_log.h"
// #include "esp_event.h"
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_gap_ble_api.h"

#include "bluetooth-SIG\assigned_numbers\core\ad_types.hpp"
#include "bluetooth-SIG\assigned_numbers\company_identifiers\company_identifiers.hpp"

namespace yabt
{
    struct BtDeviceAddrSpan;

    // Lightweight wrapper around a 6-byte Bluetooth device address.
    //
    // Purpose:
    // - Provides convenient construction/assignment from esp_bd_addr_t and from BtDeviceAddrSpan.
    // - Implements comparisons (<, ==, !=) so it can be used as key in std::map/std::set.
    // - Adds toString() helper for human-readable address formatting.
    //
    // Notes:
    // - Inherits std::array<uint8_t,6> to keep a small, POD-like layout.
    // - Comparison operators are implemented lexicographically (MSB-first) to give deterministic ordering.
    // - Use BtDeviceAddrSpan when you want a non-owning view of a 6-byte address (no copy).
    struct BtDeviceAddr : public std::array<uint8_t, 6>
    {
        using std::array<uint8_t, 6>::array;

        BtDeviceAddr &operator=(const esp_bd_addr_t &addr);
        BtDeviceAddr &operator=(const esp_bd_addr_t *addr);

        explicit BtDeviceAddr(const BtDeviceAddrSpan &span);
        BtDeviceAddr &operator=(const BtDeviceAddrSpan &span);

        bool operator==(const esp_bd_addr_t &addr) const;
        bool operator!=(const esp_bd_addr_t &addr) const;

        bool operator==(const BtDeviceAddrSpan &span) const;
        bool operator!=(const BtDeviceAddrSpan &span) const;

        bool operator<(const BtDeviceAddr &other) const;
        bool operator<(const BtDeviceAddrSpan &span) const;

        std::string toString() const;
    };

    // Lightweight non-owning view of a 6-byte Bluetooth device address.
    //
    // Purpose:
    // - Wraps a std::span<const uint8_t,6> to avoid copying address bytes.
    // - Constructible from esp_bd_addr_t (by reference or pointer).
    // - Implements lexicographic comparisons (<, ==, !=) and toString() for use in maps and logging.
    // - Use BtDeviceAddrSpan when you need a cheap, temporary view into existing address memory
    //   (for example, addresses owned by BLE report structures) without allocating or copying.
    struct BtDeviceAddrSpan
    {
        std::span<const uint8_t, 6> data;

        explicit BtDeviceAddrSpan(const esp_bd_addr_t &addr) : data(addr) {}
        explicit BtDeviceAddrSpan(const esp_bd_addr_t *addr) : data(*addr) {}

        bool operator==(const esp_bd_addr_t &addr) const;
        bool operator!=(const esp_bd_addr_t &addr) const;

        bool operator==(const BtDeviceAddr &addr) const;
        bool operator!=(const BtDeviceAddr &addr) const;

        bool operator<(const BtDeviceAddrSpan &other) const;
        bool operator<(const BtDeviceAddr &addr) const;

        std::string toString() const;
    };

    /// @brief Все данные из esp_ble_gap_ext_adv_report_t в т.ч. полученные из process_adv_data.
    ///      Формируется из esp_ble_gap_ext_adv_report_t.
    ///     Используется при поиске, если адрес неизвестен и надо передать данные в поисковые обработчики .
    /// TODO List type data in map. Check can has function.
    ///
    /// Lightweight wrapper around esp_ble_gap_ext_adv_report_t that:
    /// - collects and indexes AD segments (stores spans into the original adv buffer),
    /// - provides convenient typed accessors (flags, names, TX power, UUID lists, manufacturer data, etc.),
    /// - formats human-readable strings for logging (address, PHY, RSSI, parsed AD types).
    ///
    /// Key points:
    /// - Construct from esp_ble_gap_ext_adv_report_t (by pointer or reference).
    /// - parsed_data holds std::span views into the report's adv_data (no deep copy) — the original report buffer
    ///   must remain valid while this object is used.
    /// - Use the typed getters (getFlags, getCompleteLocalName, getManufacturerId, get16BitServiceUUIDs, ...)
    ///   in recognizers or logging code to implement device detection and event creation.
    ///
    /// Typical usage:
    ///   BleGapExtAdvReport rep(param->ext_adv_report.params);
    ///   if (myRecognizer.CanHandle(rep)) { myRecognizer.SendEvent(loop, rep); }
    ///
    /// Notes:
    /// - Parsing is defensive: getters return std::optional when data is missing or has unexpected size.
    /// - Keep instances short-lived or ensure the original esp_ble_gap_ext_adv_report_t and its adv_data live long enough.
    class BleGapExtAdvReport
    {
    private:
        const esp_ble_gap_ext_adv_report_t *raw_report;
        const BtDeviceAddrSpan addr_;
        std::map<esp_ble_adv_data_type, std::span<const uint8_t>> parsed_data;

#pragma region Common

        template <typename T>
        std::optional<T> getSingleValue(esp_ble_adv_data_type type) const;

        std::optional<std::string> getString(esp_ble_adv_data_type type) const;

        template <typename T>
        std::vector<T> getUUIDs(esp_ble_adv_data_type type) const;

        std::optional<std::span<const uint8_t>> getRawData(esp_ble_adv_data_type type) const;

        std::string getAddrTypeStr(uint8_t addr_type) const;

        std::string getPhyStr(uint8_t phy) const;

        //    // Универсальный метод для получения 1-байтовых и 2-байтовых значений
        //    template <typename T>
        //    std::optional<T> getSingleValue(esp_ble_adv_data_type type) const
        //    {
        //        auto it = parsed_data.find(type);
        //        if (it != parsed_data.end() && it->second.size() == sizeof(T))
        //        {
        //            T value;
        //            std::memcpy(&value, it->second.data(), sizeof(T));
        //            return value;
        //        }
        //        return std::nullopt;
        //    }
        //
        //    // Получение строки
        //    std::optional<std::string> getString(esp_ble_adv_data_type type) const
        //    {
        //        auto it = parsed_data.find(type);
        //        if (it != parsed_data.end() && !it->second.empty())
        //        {
        //            return std::string(reinterpret_cast<const char *>(it->second.data()), it->second.size());
        //        }
        //        return std::nullopt;
        //    }
        //
        //    // Универсальный метод для получения UUID разных размеров
        //    template <typename T>
        //    std::vector<T> getUUIDs(esp_ble_adv_data_type type) const
        //    {
        //        std::vector<T> uuids;
        //        auto it = parsed_data.find(type);
        //        if (it != parsed_data.end() && it->second.size() % sizeof(T) == 0)
        //        {
        //            size_t count = it->second.size() / sizeof(T);
        //            const uint8_t *data = it->second.data();
        //            for (size_t i = 0; i < count; ++i)
        //            {
        //                T uuid;
        //                std::memcpy(&uuid, data + i * sizeof(T), sizeof(T));
        //                uuids.push_back(uuid);
        //            }
        //        }
        //        return uuids;
        //    }
        //
        //    // Получение "сырых" данных, если нужен доступ ко всему блоку
        //    std::optional<std::span<const uint8_t>> getRawData(esp_ble_adv_data_type type) const
        //    {
        //        auto it = parsed_data.find(type);
        //        if (it != parsed_data.end() && !it->second.empty())
        //        {
        //            return it->second;
        //        }
        //        return std::nullopt;
        //    }
        //
        //    std::string getAddrTypeStr(uint8_t addr_type) const
        //    {
        //        switch (addr_type)
        //        {
        //        case BLE_ADDR_TYPE_PUBLIC:
        //            return "Public address";
        //        case BLE_ADDR_TYPE_RANDOM:
        //            return "Random address";
        //        case BLE_ADDR_TYPE_RPA_PUBLIC:
        //            return "Resolvable public";
        //        case BLE_ADDR_TYPE_RPA_RANDOM:
        //            return "Resolvable random";
        //        default:
        //            return "Unknown(" + std::to_string(addr_type) + ")";
        //        }
        //    }
        //
        //    std::string getPhyStr(uint8_t phy) const
        //    {
        //        switch (phy)
        //        {
        //        case 0:
        //            return "No preference";
        //        case ESP_BLE_GAP_PHY_1M:
        //            return "1 Mbps";
        //        case ESP_BLE_GAP_PHY_2M:
        //            return "2 Mbps";
        //        case ESP_BLE_GAP_PHY_CODED:
        //            return "Coded signal";
        //        default:
        //            return "Unknown(" + std::to_string(phy) + ")";
        //        }
        //    }

        // TODO
        template <typename T>
        std::string joinUUIDs(const std::vector<T> &uuids) const;

        std::string uuidToHexString(uint16_t uuid) const;
        std::string uuidToHexString(uint32_t uuid) const;
        std::string uuidToHexString(const std::array<uint8_t, 16> &uuid) const;

#pragma endregion

    public:
        explicit BleGapExtAdvReport(const esp_ble_gap_ext_adv_report_t &report) : BleGapExtAdvReport(&report) {}
        explicit BleGapExtAdvReport(const esp_ble_gap_ext_adv_report_t *report);
        //    : raw_report(report), addr_(report->addr)
        //{
        //    std::span<const uint8_t> adv_data((uint8_t *)raw_report->adv_data, raw_report->adv_data_len);
        //    auto iterator = adv_data.begin();
        //    auto end = adv_data.end();
        //
        //    while (iterator < end /*raw_report->adv_data_len*/)
        //    {
        //        size_t length = static_cast<size_t>(*iterator); // Длина данных в данном сегменте
        //        if (std::distance(iterator, end) < (length + 1))
        //            break; // Проверка на выход за границы
        //        if (length == 0)
        //            break;
        //
        //        esp_ble_adv_data_type type = static_cast<esp_ble_adv_data_type>(*(iterator + 1)); // Тип данных (следующий байт после длины)
        //
        //        // Добавляем в карту
        //        parsed_data[type] = std::span<const uint8_t>(iterator + 2, length - 1); // data_segment;
        //
        //        // Перемещаемся к следующему сегменту
        //        iterator += length + 1;
        //    }
        //}

#pragma region Main

        const BtDeviceAddrSpan &getAddr() const { return addr_; }

        // TODO ??? IDF Version?
        std::string getEventTypeStr() const;
        //{
        //    switch (raw_report->event_type)
        //    {
        //    case ADV_TYPE_IND:
        //        return "Connectable, Scannable";
        //    case ADV_TYPE_DIRECT_IND_HIGH:
        //        return "Directed, High Duty";
        //    case ADV_TYPE_SCAN_IND:
        //        return "Scannable";
        //    case ADV_TYPE_NONCONN_IND:
        //        return "Non-connectable";
        //    case ADV_TYPE_DIRECT_IND_LOW:
        //        return "Directed, Low Duty";
        //
        //    case ESP_BLE_LEGACY_ADV_TYPE_IND:
        //        return "ESP_BLE_LEGACY_ADV_TYPE_IND";
        //    case ESP_BLE_LEGACY_ADV_TYPE_DIRECT_IND:
        //        return "ESP_BLE_LEGACY_ADV_TYPE_DIRECT_IND";
        //    case ESP_BLE_LEGACY_ADV_TYPE_SCAN_IND:
        //        return "ESP_BLE_LEGACY_ADV_TYPE_SCAN_IND";
        //    case ESP_BLE_LEGACY_ADV_TYPE_NONCON_IND:
        //        return "ESP_BLE_LEGACY_ADV_TYPE_NONCON_IND";
        //    case ESP_BLE_LEGACY_ADV_TYPE_SCAN_RSP_TO_ADV_IND:
        //        return "ESP_BLE_LEGACY_ADV_TYPE_SCAN_RSP_TO_ADV_IND";
        //    case ESP_BLE_LEGACY_ADV_TYPE_SCAN_RSP_TO_ADV_SCAN_IND:
        //        return "ESP_BLE_LEGACY_ADV_TYPE_SCAN_RSP_TO_ADV_SCAN_IND";
        //
        //    default:
        //        return "Unknown(" + std::to_string(raw_report->event_type) + ")";
        //    }
        //}

        std::string getAddrTypeStr() const { return getAddrTypeStr(raw_report->addr_type); }

        std::string getPrimaryPhyStr() const { return getPhyStr(raw_report->primary_phy); }

        std::string getSecondlyPhyStr() const { return getPhyStr(raw_report->secondly_phy); }

        std::string getSidStr() const { return std::to_string(raw_report->sid); }

        std::string getTxPowerStr() const { return std::to_string(raw_report->tx_power) + " dBm"; }

        std::string getRssiStr() const { return std::to_string(raw_report->rssi) + " dBm"; }

        std::string getPerAdvIntervalStr() const;
        //{
        //    float interval_ms = raw_report->per_adv_interval * 1.25f;
        //    std::ostringstream oss;
        //    oss << raw_report->per_adv_interval << " (" << interval_ms << " ms)";
        //    return oss.str();
        //}

        std::string getDirAddrTypeStr() { return getAddrTypeStr(raw_report->dir_addr_type); }

        std::string getDirAddrStr() const;
        //{
        //    BtDeviceAddrSpan s(raw_report->dir_addr);
        //    return s.toString();
        //}

        std::string getDataStatusStr() const;
        //{
        //    switch (raw_report->data_status)
        //    {
        //    case ESP_BLE_GAP_EXT_ADV_DATA_COMPLETE:
        //        return "Data complete";
        //    case ESP_BLE_GAP_EXT_ADV_DATA_INCOMPLETE:
        //        return "Data partial";
        //    case ESP_BLE_GAP_EXT_ADV_DATA_TRUNCATED:
        //        return "Data cut";
        //    default:
        //        return "Unknown(" + std::to_string(raw_report->data_status) + ")";
        //    }
        //}

#pragma endregion

        std::string getMapKeysAsString() const
        {
            std::ostringstream oss;
            bool first = true;

            for (const auto &[key, value] : parsed_data)
            {
                if (!first)
                    oss << ", ";

                oss << get_ad_types_name(static_cast<uint8_t>(key));

                first = false;
            }

            return oss.str();
        }
#pragma region Flags

        // Получение флагов (ESP_BLE_AD_TYPE_FLAG)
        // TODO flag decode to bool
        std::optional<uint8_t> getFlags() const
        {
            return getSingleValue<uint8_t>(ESP_BLE_AD_TYPE_FLAG);
        }

        /// @brief Indicates that the device is in Limited Discoverable Mode. In this mode, the device broadcasts
        ///        its presence for a short, predefined period (typically up to 180 seconds) and is discoverable
        ///        only by devices actively scanning for it. This flag is used when the device wants to be
        ///        temporarily discoverable, often for initial pairing or connection purposes.
        /// @return
        std::optional<uint8_t> getFlagIsLimitedDiscoverable() const
        {
            auto flags = getFlags();
            return flags.has_value() ? std::optional<bool>((flags.value() & ESP_BLE_ADV_FLAG_LIMIT_DISC) != 0) : std::nullopt;
        }

        /// @brief Indicates that the device is in General Discoverable Mode. In this mode, the device broadcasts
        ///        its presence continuously (or until explicitly stopped) and can be discovered by any scanning
        ///        device without time restrictions. This is typically used for devices that need to remain
        ///        discoverable for extended periods, such as beacons or peripherals awaiting connections.
        /// @return std::optional<bool> - True if the flag is set, false if not, or nullopt if flags are unavailable.
        std::optional<bool> getFlagIsGeneralDiscoverable() const
        {
            auto flags = getFlags();
            return flags.has_value() ? std::optional<bool>((flags.value() & ESP_BLE_ADV_FLAG_GEN_DISC) != 0) : std::nullopt;
        }

        /// @brief Indicates that the device does not support Bluetooth Basic Rate/Enhanced Data Rate (BR/EDR),
        ///        also known as classic Bluetooth. This flag informs other devices that this peripheral operates
        ///        exclusively in Bluetooth Low Energy (BLE) mode and cannot establish connections using classic
        ///        Bluetooth protocols. Commonly set by BLE-only devices like the ESP32-S3.
        /// @return std::optional<bool> - True if the flag is set, false if not, or nullopt if flags are unavailable.
        std::optional<bool> getFlagIsBrEdrNotSupported() const
        {
            auto flags = getFlags();
            return flags.has_value() ? std::optional<bool>((flags.value() & ESP_BLE_ADV_FLAG_BREDR_NOT_SPT) != 0) : std::nullopt;
        }

        /// @brief Indicates that the device's controller supports Dual Mode operation, meaning it can handle both
        ///        Bluetooth Low Energy (BLE) and Bluetooth BR/EDR (classic Bluetooth) protocols. This flag is set
        ///        by devices with hardware capable of switching between BLE and BR/EDR, depending on the use case
        ///        or connection requirements.
        /// @return std::optional<bool> - True if the flag is set, false if not, or nullopt if flags are unavailable.
        std::optional<bool> getFlagIsDualModeControllerSupported() const
        {
            auto flags = getFlags();
            return flags.has_value() ? std::optional<bool>((flags.value() & ESP_BLE_ADV_FLAG_DMT_CONTROLLER_SPT) != 0) : std::nullopt;
        }

        /// @brief Indicates that the host stack of the device supports Dual Mode operation, enabling it to manage
        ///        both Bluetooth Low Energy (BLE) and Bluetooth BR/EDR connections. This flag complements the
        ///        controller support flag and is used by devices where the software stack is configured to handle
        ///        both BLE and classic Bluetooth interactions.
        /// @return std::optional<bool> - True if the flag is set, false if not, or nullopt if flags are unavailable.
        std::optional<bool> getFlagIsDualModeHostSupported() const
        {
            auto flags = getFlags();
            return flags.has_value() ? std::optional<bool>((flags.value() & ESP_BLE_ADV_FLAG_DMT_HOST_SPT) != 0) : std::nullopt;
        }

        /// @brief Indicates that the device is not in Limited Discoverable Mode (default state when the
        ///        ESP_BLE_ADV_FLAG_LIMIT_DISC flag is not set). This does not inherently mean general discoverability;
        ///        it simply implies the absence of the limited discovery restriction. Typically used as a fallback
        ///        when no specific discoverability mode is explicitly defined.
        /// @return std::optional<bool> - True if the device is not in limited discoverable mode, false if it is,
        ///         or nullopt if flags are unavailable.
        std::optional<bool> getFlagIsNonLimitedDiscoverable() const
        {
            auto flags = getFlags();
            return flags.has_value() ? std::optional<bool>((flags.value() & ESP_BLE_ADV_FLAG_LIMIT_DISC) == 0) : std::nullopt;
        }

        /// @brief Generates a string containing short descriptions of active BLE advertising flags,
        ///        separated by commas. Only flags that are present and supported are included.
        /// @return A string with short descriptions (e.g., "Limited discoverable, No BR/EDR support")
        ///         or an empty string if no flags are available or active.
        std::optional<std::string> getActiveFlagsDescription() const
        {
            auto flags = getFlags();
            if (!flags.has_value())
            {
                return std::nullopt;
            }

            uint8_t flagValue = flags.value();
            std::ostringstream oss; // Буфер для сборки строки
            bool first = true;      // Флаг для управления запятыми

            // Проверяем каждый флаг напрямую
            if (flagValue & ESP_BLE_ADV_FLAG_LIMIT_DISC)
            {
                oss << "Limited discoverable";
                first = false;
            }

            if (flagValue & ESP_BLE_ADV_FLAG_GEN_DISC)
            {
                if (!first)
                    oss << ", ";
                oss << "General discoverable";
                first = false;
            }

            if (flagValue & ESP_BLE_ADV_FLAG_BREDR_NOT_SPT)
            {
                if (!first)
                    oss << ", ";
                oss << "No BR/EDR support";
                first = false;
            }

            if (flagValue & ESP_BLE_ADV_FLAG_DMT_CONTROLLER_SPT)
            {
                if (!first)
                    oss << ", ";
                oss << "Dual-mode controller";
                first = false;
            }

            if (flagValue & ESP_BLE_ADV_FLAG_DMT_HOST_SPT)
            {
                if (!first)
                    oss << ", ";
                oss << "Dual-mode host";
                first = false;
            }

            if (!(flagValue & ESP_BLE_ADV_FLAG_LIMIT_DISC))
            { // NON_LIMIT_DISC — отсутствие LIMIT_DISC
                if (!first)
                    oss << ", ";
                oss << "Non-limited discoverable";
                first = false;
            }

            // Генерируем финальную строку один раз в конце
            return oss.str();
        }

#pragma endregion

        // Получение уровня TX Power (ESP_BLE_AD_TYPE_TX_PWR)
        std::optional<int8_t> getTxPowerLevel() const
        {
            return getSingleValue<int8_t>(ESP_BLE_AD_TYPE_TX_PWR);
        }

        // Получение полного имени устройства (ESP_BLE_AD_TYPE_NAME_CMPL)
        std::optional<std::string> getCompleteLocalName() const
        {
            return getString(ESP_BLE_AD_TYPE_NAME_CMPL);
        }

        // Получение сокращенного имени устройства (ESP_BLE_AD_TYPE_NAME_SHORT)
        std::optional<std::string> getShortenedLocalName() const
        {
            return getString(ESP_BLE_AD_TYPE_NAME_SHORT);
        }

        // Получение UUID сервисов (16, 32, 128 бит)
        std::vector<uint16_t> get16BitServiceUUIDs(bool complete = false) const { return getUUIDs<uint16_t>(complete ? ESP_BLE_AD_TYPE_16SRV_CMPL : ESP_BLE_AD_TYPE_16SRV_PART); }
        std::vector<uint32_t> get32BitServiceUUIDs(bool complete = false) const { return getUUIDs<uint32_t>(complete ? ESP_BLE_AD_TYPE_32SRV_CMPL : ESP_BLE_AD_TYPE_32SRV_PART); }
        std::vector<std::array<uint8_t, 16>> get128BitServiceUUIDs(bool complete = false) const { return getUUIDs<std::array<uint8_t, 16>>(complete ? ESP_BLE_AD_TYPE_128SRV_CMPL : ESP_BLE_AD_TYPE_128SRV_PART); }

        // Получение UUID сервисов (16, 32, 128 бит)
        std::vector<uint16_t> get16BitSolServiceUUIDs() const { return getUUIDs<uint16_t>(ESP_BLE_AD_TYPE_SOL_SRV_UUID); }
        std::vector<std::array<uint8_t, 16>> get128BitSolServiceUUIDs() const { return getUUIDs<std::array<uint8_t, 16>>(ESP_BLE_AD_TYPE_128SOL_SRV_UUID); }

        // TODO to options. String join optimize
        std::string get16BitServiceUUIDsAsString(bool complete = false) const { return joinUUIDs(get16BitServiceUUIDs(complete)); }
        std::string get32BitServiceUUIDsAsString(bool complete = false) const { return joinUUIDs(get32BitServiceUUIDs(complete)); }
        std::string get128BitServiceUUIDsAsString(bool complete = false) const { return joinUUIDs(get128BitServiceUUIDs(complete)); }

        // Конвертация в строку
        std::string get16BitSolServiceUUIDsAsString() const { return joinUUIDs(get16BitSolServiceUUIDs()); }
        std::string get128BitSolServiceUUIDsAsString() const { return joinUUIDs(get128BitSolServiceUUIDs()); }

        // 0x00 — Central.
        // 0x01 — Peripheral.
        std::optional<uint8_t> getTypeLeRole() const
        {
            return getSingleValue<uint8_t>(ESP_BLE_AD_TYPE_LE_ROLE);
        }

        std::optional<std::span<const uint8_t, 3>> getDeviceClass() const
        {
            auto it = parsed_data.find(ESP_BLE_AD_TYPE_DEV_CLASS);
            if (it != parsed_data.end() && it->second.size() == 3)
            {
                return std::span<const uint8_t, 3>(it->second);
            }
            return std::nullopt;
        }

        std::optional<std::span<const uint8_t, 16>> get_sm_tk_data(const std::map<esp_ble_adv_data_type, std::span<const uint8_t>> &parsed_data)
        {
            auto it = parsed_data.find(ESP_BLE_AD_TYPE_SM_TK);
            if (it != parsed_data.end() && it->second.size() == 16)
            {
                return std::span<const uint8_t, 16>(it->second); // Возвращаем span на 16 байт данных
            }
            return std::nullopt; // если данных нет или размер неправильный
        }

        std::optional<uint8_t> get_sm_oob_flag(const std::map<esp_ble_adv_data_type, std::span<const uint8_t>> &parsed_data)
        {
            return getSingleValue<uint8_t>(ESP_BLE_AD_TYPE_SM_OOB_FLAG);
        }

        std::optional<std::pair<uint8_t, uint8_t>> get_int_range(const std::map<esp_ble_adv_data_type, std::span<const uint8_t>> &parsed_data)
        {
            auto it = parsed_data.find(ESP_BLE_AD_TYPE_INT_RANGE);
            if (it != parsed_data.end() && it->second.size() == 2)
            {
                return std::make_pair(it->second[0], it->second[1]); // Возвращаем пару байт: минимум и максимум интервала
            }
            return std::nullopt; // если данных нет или размер неправильный
        }

        // Получение Appearance (ESP_BLE_AD_TYPE_APPEARANCE)
        // TODO string represenation from YAML
        std::optional<uint16_t> getAppearance() const
        {
            return getSingleValue<uint16_t>(ESP_BLE_AD_TYPE_APPEARANCE);
        }

        // Получение Advertising Interval (ESP_BLE_AD_TYPE_ADV_INT)
        // TODO  32 * 0.625 ms = 20 ms
        std::optional<uint16_t> getAdvertisingInterval() const
        {
            return getSingleValue<uint16_t>(ESP_BLE_AD_TYPE_ADV_INT);
        }

        // Получение Manufacturer Specific Data (ESP_BLE_AD_MANUFACTURER_SPECIFIC_TYPE)
        std::optional<std::span<const uint8_t>> getManufacturerData() const
        {
            return getRawData(ESP_BLE_AD_MANUFACTURER_SPECIFIC_TYPE);
        }

        std::optional<uint16_t> getManufacturerId() const
        {
            auto data = getManufacturerData();
            if (data.has_value())
            {
                uint16_t value = (((uint16_t)data.value()[0])) | (((uint16_t)data.value()[1]) << 8);
                return value;
            }
            return std::nullopt;
        }

        std::optional<std::string> getManufacturerName() const
        {
            auto id = getManufacturerId();
            if (id.has_value())
            {
                return get_company_identifiers_name(id.value());
            }
            return std::nullopt;
        }

#pragma region TODO


        //  @brief Extracts payload from BLE Service Data (AD type 0x16).
        //
        //  @param uuid
        //      16-bit service UUID to match (e.g., 0xFD5A for Samsung SmartTag).
        //      The byte order of the UUID in the advertising data is little-endian.
        //
        //  @returns
        //      std::optional<std::span<const uint8_t>> — a view to the payload
        //      (excluding UUID) if the block is found and UUID matches,
        //      otherwise std::nullopt.
        //
        //  Purpose:
        //      Returns the payload (useful data) from a BLE advertising block of type
        //      "Service Data" (AD Type = 0x16, ESP_BLE_AD_TYPE_SERVICE_DATA)
        //      for the specified 16-bit service UUID.
        //
        //  Service Data AD structure (AD Type 0x16):
        //      [ UUID_L | UUID_H | Payload... ]
        //
        //      UUID_L, UUID_H  — 16-bit service UUID in little-endian format
        //      Payload         — custom data defined by the manufacturer
        //
        //  Example (Samsung SmartTag EI-T5300):
        //      Data: 5A FD 10 43 15 00 04 42 7F 9A 0E 62 20 07 10 01
        //            |____|  |______________________________________|
        //             UUID         Payload (TagState, Battery, Flags, ...)
        //
        //      UUID 0xFD5A → Samsung SmartTag service
        //
        //
        //  Usage example:
        //      auto payload = report.getServiceDataPayload(0xFD5A);
        //      if (payload) { parseSmartTag(payload.value()); }
        //
        //  Note:
        //      For other UUID lengths, different AD types exist:
        //          • ESP_BLE_AD_TYPE_32SERVICE_DATA  (0x20)
        //          • ESP_BLE_AD_TYPE_128SERVICE_DATA (0x21)
        //      In those cases, replace subspan(2) with subspan(4) or subspan(16).
        // -----------------------------------------------------------------------------
        std::optional<std::span<const uint8_t>> getServiceDataPayload(uint16_t uuid) const
        {
            // Find the Advertising Data block with type 0x16 (Service Data, 16-bit UUID)
            auto rawOpt = getRawData(ESP_BLE_AD_TYPE_SERVICE_DATA);
            if (!rawOpt.has_value())
                return std::nullopt;

            auto span = rawOpt.value();
            if (span.size() <= 2)
                return std::nullopt; // not enough data even for UUID

            // Check 16-bit UUID (little-endian order)
            uint16_t svcUuid = static_cast<uint16_t>(span[0] | (span[1] << 8));
            if (svcUuid != uuid)
                return std::nullopt;

            // Return only the payload (skip UUID bytes)
            // subspan(2) means "start from byte #2 and go to the end"
            return span.subspan(2);
        }

        // ESP_BLE_AD_TYPE_PUBLIC_TARGET
        // ESP_BLE_AD_TYPE_RANDOM_TARGET
        // ESP_BLE_AD_TYPE_LE_DEV_ADDR
        // ESP_BLE_AD_TYPE_SPAIR_C256
        // ESP_BLE_AD_TYPE_SPAIR_R256
        // ESP_BLE_AD_TYPE_32SOL_SRV_UUID
        // ESP_BLE_AD_TYPE_32SERVICE_DATA
        // ESP_BLE_AD_TYPE_128SERVICE_DATA
        // ESP_BLE_AD_TYPE_LE_SECURE_CONFIRM
        // ESP_BLE_AD_TYPE_LE_SECURE_RANDOM
        // ESP_BLE_AD_TYPE_URI
        // ESP_BLE_AD_TYPE_INDOOR_POSITION
        // ESP_BLE_AD_TYPE_TRANS_DISC_DATA
        // ESP_BLE_AD_TYPE_LE_SUPPORT_FEATURE
        // ESP_BLE_AD_TYPE_CHAN_MAP_UPDATE

#pragma endregion
    };

} // namespace yabt
