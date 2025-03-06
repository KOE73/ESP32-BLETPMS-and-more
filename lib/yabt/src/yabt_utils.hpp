

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

#include "bluetooth-SIG\ad_types.hpp"
#include "bluetooth-SIG\company_identifiers.h"

namespace yabt
{
    struct BtDeviceAddrSpan;

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

        std::string toString() const;
    };

    struct BtDeviceAddrSpan
    {
        std::span<const uint8_t, 6> data;

        explicit BtDeviceAddrSpan(const esp_bd_addr_t &addr) : data(addr) {}
        explicit BtDeviceAddrSpan(const esp_bd_addr_t *addr) : data(*addr) {}

        bool operator==(const esp_bd_addr_t &addr) const;
        bool operator!=(const esp_bd_addr_t &addr) const;

        bool operator==(const BtDeviceAddr &addr) const;
        bool operator!=(const BtDeviceAddr &addr) const;

        std::string toString() const;
    };

    // struct BtDeviceAddr : public std::array<uint8_t, 6>
    //{
    //     using std::array<uint8_t, 6>::array; // Наследуем конструкторы std::array
    //
    //    explicit BtDeviceAddr(const BtDeviceAddrSpan &span)
    //    {
    //        std::copy(span.data.begin(), span.data.end(), this->begin());
    //    }
    //
    //    BtDeviceAddr &operator=(const BtDeviceAddrSpan &span)
    //    {
    //        std::copy(span.data.begin(), span.data.end(), this->begin());
    //        return *this;
    //    }
    //
    //    BtDeviceAddr &operator=(const esp_bd_addr_t &addr)
    //    {
    //        std::copy(std::begin(addr), std::end(addr), this->begin());
    //        return *this;
    //    }
    //
    //    BtDeviceAddr &operator=(const esp_bd_addr_t *addr)
    //    {
    //        std::copy(std::begin(*addr), std::end(*addr), this->begin());
    //        return *this;
    //    }
    //
    //    bool operator==(const esp_bd_addr_t &addr) const
    //    {
    //        return std::equal(this->begin(), this->end(), std::begin(addr));
    //    }
    //
    //    bool operator!=(const esp_bd_addr_t &addr) const
    //    {
    //        return !(*this == addr);
    //    }
    //
    //    bool operator==(const BtDeviceAddrSpan &span) const
    //    {
    //        return std::equal(this->begin(), this->end(), span.data.begin());
    //    }
    //
    //    bool operator!=(const BtDeviceAddrSpan &span) const
    //    {
    //        return !(*this == span);
    //    }
    //
    //    std::string toString() const
    //    {
    //        std::ostringstream oss;
    //        oss << std::hex << std::uppercase << std::setfill('0'); // Настраиваем формат: HEX, заглавные буквы, заполнение нулями
    //
    //        // Используем итераторы для обхода массива
    //        for (auto it = begin(); it != end(); ++it)
    //        {
    //            oss << std::setw(2) << static_cast<unsigned>(*it);
    //            if (it != end() - 1)
    //                oss << ":";
    //        }
    //
    //        return oss.str();
    //    }
    //};
    //
    // struct BtDeviceAddrSpan
    //{
    //    std::span<const uint8_t, 6> data;
    //
    //    explicit BtDeviceAddrSpan(const esp_bd_addr_t &addr) : data(addr) {}
    //    explicit BtDeviceAddrSpan(const esp_bd_addr_t *addr) : data(*addr) {}
    //
    //    bool operator==(const esp_bd_addr_t &addr) const
    //    {
    //        return std::equal(data.begin(), data.end(), std::begin(addr));
    //    }
    //
    //    bool operator!=(const esp_bd_addr_t &addr) const
    //    {
    //        return !(*this == addr);
    //    }
    //
    //    bool operator==(const BtDeviceAddr &addr) const
    //    {
    //        return std::equal(data.begin(), data.end(), addr.begin());
    //    }
    //
    //    bool operator!=(const BtDeviceAddr &addr) const
    //    {
    //        return !(*this == addr);
    //    }
    //
    //    /// @brief Formats the Bluetooth device address as a string in the format "XX:XX:XX:XX:XX:XX".
    //    ///        Each byte is converted to a two-digit hexadecimal value, separated by colons.
    //    /// @return A string representation of the Bluetooth address.
    //    std::string toString() const
    //    {
    //        std::ostringstream oss;
    //        oss << std::hex << std::uppercase << std::setfill('0'); // HEX, заглавные, нули
    //
    //        for (auto it = data.begin(); it != data.end(); ++it)
    //        {
    //            oss << std::setw(2) << static_cast<unsigned>(*it);
    //            if (it != data.end() - 1)
    //                oss << ":";
    //        }
    //
    //        return oss.str();
    //    }
    //};

    /// @bri Все данные из esp_ble_gap_ext_adv_report_t в т.ч. полученные из process_adv_data.
    ///      Формируется из esp_ble_gap_ext_adv_report_t.
    ///     Используется при поиске, если адрес неизвестен и надо передать данные в поисковые обработчики .
    // TODO List type data in map. Check can has function.
    class BleGapExtAdvReport
    {
    private:
        const esp_ble_gap_ext_adv_report_t *raw_report;
        const BtDeviceAddrSpan addr_;
        std::map<esp_ble_adv_data_type, std::span<const uint8_t>> parsed_data;

        // Универсальный метод для получения 1-байтовых и 2-байтовых значений
        template <typename T>
        std::optional<T> getSingleValue(esp_ble_adv_data_type type) const
        {
            auto it = parsed_data.find(type);
            if (it != parsed_data.end() && it->second.size() == sizeof(T))
            {
                T value;
                std::memcpy(&value, it->second.data(), sizeof(T));
                return value;
            }
            return std::nullopt;
        }

        // Получение строки
        std::optional<std::string> getString(esp_ble_adv_data_type type) const
        {
            auto it = parsed_data.find(type);
            if (it != parsed_data.end() && !it->second.empty())
            {
                return std::string(reinterpret_cast<const char *>(it->second.data()), it->second.size());
            }
            return std::nullopt;
        }

        // Универсальный метод для получения UUID разных размеров
        template <typename T>
        std::vector<T> getUUIDs(esp_ble_adv_data_type type) const
        {
            std::vector<T> uuids;
            auto it = parsed_data.find(type);
            if (it != parsed_data.end() && it->second.size() % sizeof(T) == 0)
            {
                size_t count = it->second.size() / sizeof(T);
                const uint8_t *data = it->second.data();
                for (size_t i = 0; i < count; ++i)
                {
                    T uuid;
                    std::memcpy(&uuid, data + i * sizeof(T), sizeof(T));
                    uuids.push_back(uuid);
                }
            }
            return uuids;
        }

        // Получение "сырых" данных, если нужен доступ ко всему блоку
        std::optional<std::span<const uint8_t>> getRawData(esp_ble_adv_data_type type) const
        {
            auto it = parsed_data.find(type);
            if (it != parsed_data.end() && !it->second.empty())
            {
                return it->second;
            }
            return std::nullopt;
        }

    public:
        explicit BleGapExtAdvReport(const esp_ble_gap_ext_adv_report_t &report) : BleGapExtAdvReport(&report) {}
        explicit BleGapExtAdvReport(const esp_ble_gap_ext_adv_report_t *report)
            : raw_report(report), addr_(report->addr)
        {
            std::span<const uint8_t> adv_data((uint8_t*)raw_report->adv_data, raw_report->adv_data_len);
            auto iterator = adv_data.begin();
            auto end = adv_data.end();

            while (iterator < end /*raw_report->adv_data_len*/)
            {
                size_t length = static_cast<size_t>(*iterator); // Длина данных в данном сегменте
                if (std::distance(iterator, end) < (length + 1))
                    break; // Проверка на выход за границы
                if (length == 0)
                    break;

                esp_ble_adv_data_type type = static_cast<esp_ble_adv_data_type>(*(iterator + 1)); // Тип данных (следующий байт после длины)

                // Добавляем в карту
                parsed_data[type] = std::span<const uint8_t>(iterator + 2, length - 1); // data_segment;

                // Перемещаемся к следующему сегменту
                iterator += length + 1;
            }
        }

        std::string getMapKeysAsString()
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

        const BtDeviceAddrSpan &getAddr() const { return addr_; }

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
        std::vector<uint16_t> get16BitServiceUUIDs(bool complete = false) const
        {
            return getUUIDs<uint16_t>(complete ? ESP_BLE_AD_TYPE_16SRV_CMPL : ESP_BLE_AD_TYPE_16SRV_PART);
        }

        std::vector<uint32_t> get32BitServiceUUIDs(bool complete = false) const
        {
            return getUUIDs<uint32_t>(complete ? ESP_BLE_AD_TYPE_32SRV_CMPL : ESP_BLE_AD_TYPE_32SRV_PART);
        }

        std::vector<std::array<uint8_t, 16>> get128BitServiceUUIDs(bool complete = false) const
        {
            return getUUIDs<std::array<uint8_t, 16>>(complete ? ESP_BLE_AD_TYPE_128SRV_CMPL : ESP_BLE_AD_TYPE_128SRV_PART);
        }

        // Получение UUID сервисов (16, 32, 128 бит)
        std::vector<uint16_t> get16BitSolServiceUUIDs(bool complete = false) const
        {
            return getUUIDs<uint16_t>(ESP_BLE_AD_TYPE_SOL_SRV_UUID);
        }

        std::vector<std::array<uint8_t, 16>> get128BitSolServiceUUIDs(bool complete = false) const
        {
            return getUUIDs<std::array<uint8_t, 16>>(ESP_BLE_AD_TYPE_128SOL_SRV_UUID);
        }

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
                return get_company_name(id.value());
            }
            return std::nullopt;
        }

#pragma region TODO
        std::optional<std::span<const uint8_t>> get16BitServiceData(bool complete = false) const
        {
            auto it = parsed_data.find(ESP_BLE_AD_TYPE_SERVICE_DATA);
            if (it != parsed_data.end() && it->second.size() == 3)
            {
                return it->second;
            }
            return std::nullopt;
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
