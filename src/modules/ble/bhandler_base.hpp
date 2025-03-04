

#include "string.h"

// #include <iostream>
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

// #include "esp_system.h"
// #include "esp_log.h"
// #include "esp_event.h"
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_gap_ble_api.h"

using bt_device_addr_t = std::array<uint8_t, 6>;

struct bt_device_addr_t : public std::array<uint8_t, 6>
{
    using std::array<uint8_t, 6>::array; // Наследуем конструкторы std::array

    // Оператор присваивания из esp_bd_addr_t (uint8_t[6])
    bt_device_addr_t &operator=(const esp_bd_addr_t &addr)
    {
        std::copy(std::begin(addr), std::end(addr), this->begin());
        return *this;
    }
    bt_device_addr_t &operator=(const esp_bd_addr_t *addr)
    {
        std::copy(std::begin(*addr), std::end(*addr), this->begin());
        return *this;
    }

    // Оператор сравнения с esp_bd_addr_t (uint8_t[6])
    bool operator==(const esp_bd_addr_t &addr) const
    {
        return std::equal(this->begin(), this->end(), std::begin(addr));
    }

    bool operator!=(const esp_bd_addr_t &addr) const
    {
        return !(*this == addr);
    }
};

// Для фабрики. Пока думаю зачем
#define REGISTER_CLASS(Derived)                                                                                                      \
    namespace                                                                                                                        \
    {                                                                                                                                \
        struct Register##Derived                                                                                                     \
        {                                                                                                                            \
            Register##Derived()                                                                                                      \
            {                                                                                                                        \
                Factory::instance().register_class(#Derived, []() -> std::unique_ptr<Base> { return std::make_unique<Derived>(); }); \
            }                                                                                                                        \
        } reg##Derived;                                                                                                              \
    }

namespace yabt
{
    class bhandler_gap_base;

    // Фабрика. Пока думаю зачем
    class Factory
    {
    public:
        using CreateFunc = std::unique_ptr<bhandler_gap_base> (*)();

        static Factory &instance()
        {
            static Factory f;
            return f;
        }

        void register_class(const std::string &name, CreateFunc func)
        {
            creators[name] = func;
        }

        std::vector<std::unique_ptr<bhandler_gap_base>> create_all()
        {
            std::vector<std::unique_ptr<bhandler_gap_base>> objects;
            for (const auto &[name, func] : creators)
            {
                objects.push_back(func());
            }
            return objects;
        }

    private:
        std::unordered_map<std::string, CreateFunc> creators;
    };

    /// @bri Все данные из esp_ble_gap_ext_adv_report_t в т.ч. полученные из process_adv_data.
    ///      Формируется из esp_ble_gap_ext_adv_report_t.
    ///     Используется при поиске, если адрес неизвестен и надо передать данные в поисковые обработчики .
    class bt_info
    {
    };

    class BleAdvReport
    {
    private:
        const esp_ble_gap_ext_adv_report_t *raw_report;
        // std::vector<uint8_t> adv_data;

        std::map<esp_ble_adv_data_type, std::span<const uint8_t>> parsed_data;

        // Универсальный метод извлечения UUID разных размеров
        template <typename T>
        std::vector<T> extractUUIDs(esp_ble_adv_data_type type) const
        {
            auto rawData = getAdvData(type);
            std::vector<T> uuids;
            if (rawData && rawData->size() % sizeof(T) == 0)
            {
                for (size_t i = 0; i < rawData->size(); i += sizeof(T))
                {
                    T uuid;
                    std::memcpy(&uuid, rawData->data() + i, sizeof(T));
                    uuids.push_back(uuid);
                }
            }
            return uuids;
        }

    public:
        explicit BleAdvReport(const esp_ble_gap_ext_adv_report_t *report)
            : raw_report(report) //,
        // adv_data(report.adv_data, report.adv_data + report.adv_data_len)
        {
            auto d = raw_report->adv_data;
            size_t index = 0;
            //uint8_t adv_data[251]; // фиксированный массив
            //std::span<uint8_t> adv_data(adv_data);
            std::span<const uint8_t> adv_data(raw_report->adv_data);
            // auto adv_data = raw_report->adv_data;

            while (index < raw_report->adv_data_len)
            {
                size_t length = (size_t)adv_data[index]; // Длина данных в данном сегменте
                if (length == 0)
                {
                    break;
                }

                esp_ble_adv_data_type type = static_cast<esp_ble_adv_data_type>(adv_data[index + 1]); // Тип данных (следующий байт после длины)

                // Создаем span для текущего сегмента данных, начиная с индекса и длины
                std::span<const uint8_t> data_segment(adv_data.begin() + index + 2, length);

                // Добавляем в карту
                parsed_data[type] = data_segment;

                // Перемещаемся к следующему сегменту
                index += length;
            }
        }

        // Получение данных ADV по BLE типу с автоматическим преобразованием в std::vector<uint8_t>
        std::optional<std::vector<uint8_t>> getAdvData(esp_ble_adv_data_type type) const
        {
            uint8_t data_len = 0;
            const uint8_t *data = esp_ble_resolve_adv_data_by_type((uint8_t *)raw_report->adv_data, raw_report->adv_data_len, type, &data_len);
            if (data && data_len > 0)
            {
                return std::vector<uint8_t>(data, data + data_len);
            }
            return std::nullopt;
        }

        // Имя устройства (если присутствует)
        std::optional<std::string> getDeviceName() const
        {
            if (auto data = getAdvData(ESP_BLE_AD_TYPE_NAME_CMPL))
            {
                return std::string(reinterpret_cast<const char *>(data->data()), data->size());
            }
            return std::nullopt;
        }

        // 16-битные UUID (полный список)
        std::vector<uint16_t> get16BitUUIDs() const
        {
            return extractUUIDs<uint16_t>(ESP_BLE_AD_TYPE_16SRV_CMPL);
        }

        // 32-битные UUID (полный список)
        std::vector<uint32_t> get32BitUUIDs() const
        {
            return extractUUIDs<uint32_t>(ESP_BLE_AD_TYPE_32SRV_CMPL);
        }

        // 128-битные UUID (полный список)
        std::vector<std::array<uint8_t, 16>> get128BitUUIDs() const
        {
            auto rawData = getAdvData(ESP_BLE_AD_TYPE_128SRV_CMPL);
            std::vector<std::array<uint8_t, 16>> uuids;
            if (rawData && rawData->size() % 16 == 0)
            {
                for (size_t i = 0; i < rawData->size(); i += 16)
                {
                    std::array<uint8_t, 16> uuid;
                    std::copy(rawData->begin() + i, rawData->begin() + i + 16, uuid.begin());
                    uuids.push_back(uuid);
                }
            }
            return uuids;
        }

        // Производительские данные
        std::optional<std::vector<uint8_t>> getManufacturerData() const
        {
            return getAdvData(ESP_BLE_AD_TYPE_MANUFACTURER_SPECIFIC);
        }

        // Вывод всех ADV-данных в консоль
        void printAdvData() const
        {
            std::cout << "RAW ADV Data: ";
            for (uint8_t byte : adv_data)
            {
                std::cout << std::hex << std::setw(2) << std::setfill('0')
                          << static_cast<int>(byte) << " ";
            }
            std::cout << std::dec << std::endl;

            if (auto name = getDeviceName())
            {
                std::cout << "Device Name: " << *name << std::endl;
            }

            auto uuids16 = get16BitUUIDs();
            if (!uuids16.empty())
            {
                std::cout << "16-bit UUIDs: ";
                for (auto uuid : uuids16)
                {
                    std::cout << "0x" << std::hex << uuid << " ";
                }
                std::cout << std::dec << std::endl;
            }

            auto uuids32 = get32BitUUIDs();
            if (!uuids32.empty())
            {
                std::cout << "32-bit UUIDs: ";
                for (auto uuid : uuids32)
                {
                    std::cout << "0x" << std::hex << uuid << " ";
                }
                std::cout << std::dec << std::endl;
            }

            auto uuids128 = get128BitUUIDs();
            if (!uuids128.empty())
            {
                std::cout << "128-bit UUIDs: ";
                for (const auto &uuid : uuids128)
                {
                    for (auto byte : uuid)
                    {
                        std::cout << std::hex << std::setw(2) << std::setfill('0')
                                  << static_cast<int>(byte);
                    }
                    std::cout << " ";
                }
                std::cout << std::dec << std::endl;
            }

            if (auto manufacturerData = getManufacturerData())
            {
                std::cout << "Manufacturer Data: ";
                for (uint8_t byte : *manufacturerData)
                {
                    std::cout << std::hex << std::setw(2) << std::setfill('0')
                              << static_cast<int>(byte) << " ";
                }
                std::cout << std::dec << std::endl;
            }
        }
    };

    void exampleUsage(const esp_ble_gap_ext_adv_report_t &report)
    {
        BleAdvReport adv(report);

        // Вывести всю информацию
        adv.printAdvData();

        // Получить имя устройства
        if (auto deviceName = adv.getDeviceName())
        {
            std::cout << "Имя устройства: " << *deviceName << std::endl;
        }

        // Получить 16-битные UUID
        auto uuids16 = adv.get16BitUUIDs();
        for (auto uuid : uuids16)
        {
            std::cout << "Найден 16-bit UUID: 0x" << std::hex << uuid << std::dec << std::endl;
        }

        // Получить 128-битные UUID
        auto uuids128 = adv.get128BitUUIDs();
        if (!uuids128.empty())
        {
            std::cout << "Найден 128-bit UUID: ";
            for (const auto &uuid : uuids128)
            {
                for (auto byte : uuid)
                {
                    std::cout << std::hex << std::setw(2) << std::setfill('0')
                              << static_cast<int>(byte);
                }
                std::cout << " ";
            }
            std::cout << std::dec << std::endl;
        }

        // Получить Manufacturer Specific Data
        if (auto mfg_data = adv.getManufacturerData())
        {
            std::cout << "Manufacturer Data: ";
            for (uint8_t byte : *mfg_data)
            {
                std::cout << std::hex << std::setw(2) << std::setfill('0')
                          << static_cast<int>(byte) << " ";
            }
            std::cout << std::dec << std::endl;
        }
    }

    /// @brief Список обработчиков с известными адресами
    std::vector<bhandler_gap_base> known_handlers;
    std::map<bt_device_addr_t, bhandler_gap_base> known_addreses;

    class bhandler_gap_base
    {
    private:
        /* data */
    public:
        bhandler_gap_base(/* args */) {};
        ~bhandler_gap_base() {};

        void handle(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param);
    };

    class bhandler_gap_addr
    {
    protected:
        bt_device_addr_t addr_;

    public:
        bhandler_gap_addr(esp_bd_addr_t *esp_addr)
        {
            addr_ = esp_addr;
        };
        ~bhandler_gap_addr() {};

        /// @brief Checking that the request can be processed. Basic by address only.
        /// @param param
        /// @return true - if can
        virtual bool canHandle(esp_ble_gap_ext_adv_report_t *param) { return addr_ == param->addr; }

        virtual bool handle(esp_ble_gap_ext_adv_report_t *param)
        {
            if (addr_ != param->addr)
                return false;
        }
    };

    // bhandler_base::bhandler_base(/* args */)
    //{
    // }
    //
    // bhandler_base::~bhandler_base()
    //{
    // }

    class bhandler_gap_tpms_tomtom : bhandler_gap_addr
    {
    public:
        bhandler_gap_tpms_tomtom(esp_bd_addr_t *esp_addr) : bhandler_gap_addr(esp_addr) {}

        bool handle(esp_ble_gap_ext_adv_report_t *param) override
        {
            if (addr_ != param->addr)
                return false;
        }
    }

} // namespace yabt
