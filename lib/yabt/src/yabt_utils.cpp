

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
#include <ranges>

// #include "esp_system.h"
// #include "esp_log.h"
// #include "esp_event.h"
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_gap_ble_api.h"

#include "bluetooth-SIG\assigned_numbers\core\ad_types.hpp"
// #include "bluetooth-SIG\ad_types.h"

#include "yabt_utils.hpp"

namespace yabt
{

    BtDeviceAddr &BtDeviceAddr::operator=(const esp_bd_addr_t &addr)
    {
        std::copy(std::begin(addr), std::end(addr), this->begin());
        return *this;
    }

    BtDeviceAddr &BtDeviceAddr::operator=(const esp_bd_addr_t *addr)
    {
        std::copy(std::begin(*addr), std::end(*addr), this->begin());
        return *this;
    }

    BtDeviceAddr::BtDeviceAddr(const BtDeviceAddrSpan &span) { std::copy(span.data.begin(), span.data.end(), this->begin()); }

    BtDeviceAddr &BtDeviceAddr::operator=(const BtDeviceAddrSpan &span)
    {
        std::copy(span.data.begin(), span.data.end(), this->begin());
        return *this;
    }

    bool BtDeviceAddr::operator==(const esp_bd_addr_t &addr) const { return std::equal(this->begin(), this->end(), std::begin(addr)); }
    bool BtDeviceAddr::operator!=(const esp_bd_addr_t &addr) const { return !(*this == addr); }

    bool BtDeviceAddr::operator==(const BtDeviceAddrSpan &span) const { return std::equal(this->begin(), this->end(), span.data.begin()); }
    bool BtDeviceAddr::operator!=(const BtDeviceAddrSpan &span) const { return !(*this == span); }

    bool BtDeviceAddr::operator<(const BtDeviceAddr &other) const { return std::lexicographical_compare(this->begin(), this->end(), other.begin(), other.end()); }
    bool BtDeviceAddr::operator<(const BtDeviceAddrSpan &span) const { return std::lexicographical_compare(this->begin(), this->end(), span.data.begin(), span.data.end()); }

    std::string BtDeviceAddr::toString() const
    {
        std::ostringstream oss;
        oss << std::hex << std::uppercase << std::setfill('0'); // Настраиваем формат: HEX, заглавные буквы, заполнение нулями

        // Используем итераторы для обхода массива
        for (auto it = begin(); it != end(); ++it)
        {
            oss << std::setw(2) << static_cast<unsigned>(*it);
            if (it != end() - 1)
                oss << ":";
        }

        return oss.str();
    }

    bool BtDeviceAddrSpan::operator==(const esp_bd_addr_t &addr) const { return std::equal(data.begin(), data.end(), std::begin(addr)); }
    bool BtDeviceAddrSpan::operator!=(const esp_bd_addr_t &addr) const { return !(*this == addr); }

    bool BtDeviceAddrSpan::operator==(const BtDeviceAddr &addr) const { return std::equal(data.begin(), data.end(), addr.begin()); }
    bool BtDeviceAddrSpan::operator!=(const BtDeviceAddr &addr) const { return !(*this == addr); }

    bool BtDeviceAddrSpan::operator<(const BtDeviceAddrSpan &other) const { return std::lexicographical_compare(this->data.begin(), this->data.end(), other.data.begin(), other.data.end()); }
    bool BtDeviceAddrSpan::operator<(const BtDeviceAddr &addr) const { return std::lexicographical_compare(this->data.begin(), this->data.end(), addr.begin(), addr.end()); }

    std::string BtDeviceAddrSpan::toString() const
    {
        std::ostringstream oss;
        oss << std::hex << std::uppercase << std::setfill('0'); // HEX, заглавные, нули

        for (auto it = data.begin(); it != data.end(); ++it)
        {
            oss << std::setw(2) << static_cast<unsigned>(*it);
            if (it != data.end() - 1)
                oss << ":";
        }

        return oss.str();
    }

#pragma region -BleGapExtAdvReport

    BleGapExtAdvReport::BleGapExtAdvReport(const esp_ble_gap_ext_adv_report_t *report)
        : raw_report(report), addr_(report->addr)
    {
        std::span<const uint8_t> adv_data((uint8_t *)raw_report->adv_data, raw_report->adv_data_len);
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

    template <typename T>
    std::optional<T> BleGapExtAdvReport::getSingleValue(esp_ble_adv_data_type type) const
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

    std::optional<std::string> BleGapExtAdvReport::getString(esp_ble_adv_data_type type) const
    {
        auto it = parsed_data.find(type);
        if (it != parsed_data.end() && !it->second.empty())
        {
            return std::string(reinterpret_cast<const char *>(it->second.data()), it->second.size());
        }
        return std::nullopt;
    }

    template <typename T>
    std::vector<T> BleGapExtAdvReport::getUUIDs(esp_ble_adv_data_type type) const
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

    // Экспорт шаблонов для конкретных типов (если нужно, уточните типы)
    template std::optional<uint8_t> BleGapExtAdvReport::getSingleValue<uint8_t>(esp_ble_adv_data_type type) const;
    template std::optional<uint16_t> BleGapExtAdvReport::getSingleValue<uint16_t>(esp_ble_adv_data_type type) const;
    template std::vector<uint16_t> BleGapExtAdvReport::getUUIDs<uint16_t>(esp_ble_adv_data_type type) const;
    template std::vector<uint32_t> BleGapExtAdvReport::getUUIDs<uint32_t>(esp_ble_adv_data_type type) const;
    template std::vector<std::array<uint8_t, 16>> BleGapExtAdvReport::getUUIDs<std::array<uint8_t, 16>>(esp_ble_adv_data_type type) const;

    // Функция объединения UUID через запятую
    template <typename T>
    std::string BleGapExtAdvReport::joinUUIDs(const std::vector<T> &uuids) const
    {
        if (uuids.empty())
            return "";

        std::ostringstream oss;
        for (size_t i = 0; i < uuids.size(); ++i)
        {
            if (i > 0)
                oss << ","; // Разделитель
            oss << uuidToHexString(uuids[i]);
        }
        return oss.str();
    }

    template std::string BleGapExtAdvReport::joinUUIDs(const std::vector<uint16_t> &uuids) const;
    template std::string BleGapExtAdvReport::joinUUIDs(const std::vector<uint32_t> &uuids) const;
    template std::string BleGapExtAdvReport::joinUUIDs(const std::vector<std::array<uint8_t, 16>> &uuids) const;

    // Конвертация UUID в строку (16-битный)
    std::string BleGapExtAdvReport::uuidToHexString(uint16_t uuid) const
    {
        std::ostringstream oss;
        oss << "0x" << std::hex << std::uppercase << std::setw(4) << std::setfill('0') << uuid;
        return oss.str();
    }

    // Конвертация UUID в строку (32-битный)
    std::string BleGapExtAdvReport::uuidToHexString(uint32_t uuid) const
    {
        std::ostringstream oss;
        oss << "0x" << std::hex << std::uppercase << std::setw(8) << std::setfill('0') << uuid;
        return oss.str();
    }

    // Конвертация UUID в строку (128-битный)
    std::string BleGapExtAdvReport::uuidToHexString(const std::array<uint8_t, 16> &uuid) const
    {
        std::ostringstream oss;
        for (size_t i = 0; i < uuid.size(); ++i)
        {
            if (i > 0)
                oss << ""; // Без разделителя в самом UUID
            oss << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << static_cast<int>(uuid[i]);
            if (i == 3 || i == 5 || i == 7 || i == 9)
            {
                oss << "-"; // Формат UUID (8-4-4-4-12)
            }
        }
        return oss.str();
    }

    std::optional<std::span<const uint8_t>> BleGapExtAdvReport::getRawData(esp_ble_adv_data_type type) const
    {
        auto it = parsed_data.find(type);
        if (it != parsed_data.end() && !it->second.empty())
        {
            return it->second;
        }
        return std::nullopt;
    }

    std::string BleGapExtAdvReport::getAddrTypeStr(uint8_t addr_type) const
    {
        switch (addr_type)
        {
        case BLE_ADDR_TYPE_PUBLIC:
            return "Public address";
        case BLE_ADDR_TYPE_RANDOM:
            return "Random address";
        case BLE_ADDR_TYPE_RPA_PUBLIC:
            return "Resolvable public";
        case BLE_ADDR_TYPE_RPA_RANDOM:
            return "Resolvable random";
        default:
            return "Unknown(" + std::to_string(addr_type) + ")";
        }
    }

    std::string BleGapExtAdvReport::getPhyStr(uint8_t phy) const
    {
        switch (phy)
        {
        case 0:
            return "No preference";
        case ESP_BLE_GAP_PHY_1M:
            return "1 Mbps";
        case ESP_BLE_GAP_PHY_2M:
            return "2 Mbps";
        case ESP_BLE_GAP_PHY_CODED:
            return "Coded signal";
        default:
            return "Unknown(" + std::to_string(phy) + ")";
        }
    }
    

    std::string BleGapExtAdvReport::getEventTypeStr() const
    {
        switch (raw_report->event_type)
        {
        case ADV_TYPE_IND:
            return "Connectable, Scannable";
        case ADV_TYPE_DIRECT_IND_HIGH:
            return "Directed, High Duty";
        case ADV_TYPE_SCAN_IND:
            return "Scannable";
        case ADV_TYPE_NONCONN_IND:
            return "Non-connectable";
        case ADV_TYPE_DIRECT_IND_LOW:
            return "Directed, Low Duty";

        case ESP_BLE_LEGACY_ADV_TYPE_IND:
            return "ESP_BLE_LEGACY_ADV_TYPE_IND";
        case ESP_BLE_LEGACY_ADV_TYPE_DIRECT_IND:
            return "ESP_BLE_LEGACY_ADV_TYPE_DIRECT_IND";
        case ESP_BLE_LEGACY_ADV_TYPE_SCAN_IND:
            return "ESP_BLE_LEGACY_ADV_TYPE_SCAN_IND";
        case ESP_BLE_LEGACY_ADV_TYPE_NONCON_IND:
            return "ESP_BLE_LEGACY_ADV_TYPE_NONCON_IND";
        case ESP_BLE_LEGACY_ADV_TYPE_SCAN_RSP_TO_ADV_IND:
            return "ESP_BLE_LEGACY_ADV_TYPE_SCAN_RSP_TO_ADV_IND";
        case ESP_BLE_LEGACY_ADV_TYPE_SCAN_RSP_TO_ADV_SCAN_IND:
            return "ESP_BLE_LEGACY_ADV_TYPE_SCAN_RSP_TO_ADV_SCAN_IND";

        default:
            return "Unknown(" + std::to_string(raw_report->event_type) + ")";
        }
    }

    std::string BleGapExtAdvReport::getPerAdvIntervalStr() const
    {
        float interval_ms = raw_report->per_adv_interval * 1.25f;
        std::ostringstream oss;
        oss << raw_report->per_adv_interval << " (" << interval_ms << " ms)";
        return oss.str();
    }

    std::string BleGapExtAdvReport::getDirAddrStr() const
    {
        BtDeviceAddrSpan s(raw_report->dir_addr);
        return s.toString();
    }

    std::string BleGapExtAdvReport::getDataStatusStr() const
    {
        switch (raw_report->data_status)
        {
        case ESP_BLE_GAP_EXT_ADV_DATA_COMPLETE:
            return "Data complete";
        case ESP_BLE_GAP_EXT_ADV_DATA_INCOMPLETE:
            return "Data partial";
        case ESP_BLE_GAP_EXT_ADV_DATA_TRUNCATED:
            return "Data cut";
        default:
            return "Unknown(" + std::to_string(raw_report->data_status) + ")";
        }
    }

#pragma endregion

    void d()
    {
        std::vector<std::string> words = {"apple", "banana", "cherry", "lada"};
        auto joined = std::views::join_with(words, ", ");
        // auto joined = std::views::join(words);

        std::ostringstream oss;
        for (char c : joined)
        {
            oss.put(c); // Используем put() вместо += (эффективнее)
        }

        std::string result = oss.str();
    }

} // namespace yabt
