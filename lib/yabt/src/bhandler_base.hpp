

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


    // TODO List type data in map. Check can has function.
    class BleAdvReport
    {
    private:
        const esp_ble_gap_ext_adv_report_t *raw_report;
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
        explicit BleAdvReport(const esp_ble_gap_ext_adv_report_t *report)
            : raw_report(report) //,
        // adv_data(report.adv_data, report.adv_data + report.adv_data_len)
        {
            auto d = raw_report->adv_data;
            std::span<const uint8_t> adv_data(raw_report->adv_data);
            auto iterator = adv_data.begin();
            auto end = adv_data.end();

            while (iterator < end /*raw_report->adv_data_len*/)
            {
                size_t length = static_cast<size_t>(*iterator); // Длина данных в данном сегменте
                if (std::distance(iterator, end) < length)
                    break; // Проверка на выход за границы
                if (length == 0)
                    break;

                esp_ble_adv_data_type type = static_cast<esp_ble_adv_data_type>(*(iterator + 1)); // Тип данных (следующий байт после длины)

                // Создаем span для текущего сегмента данных, начиная с индекса и длины
                // std::span<const uint8_t> data_segment(iterator + 2, length);

                // Добавляем в карту
                parsed_data[type] = std::span<const uint8_t>(iterator + 2, length); // data_segment;

                // Перемещаемся к следующему сегменту
                iterator += length;
            }
        }

        // Получение флагов (ESP_BLE_AD_TYPE_FLAG)
        // TODO flag decode to bool
        std::optional<uint8_t> getFlags() const
        {
            return getSingleValue<uint8_t>(ESP_BLE_AD_TYPE_FLAG);
        }


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

        #pragma region TODO  
        std::optional<std::span<const uint8_t>> get16BitServiceData (bool complete = false) const
        {
            auto it = parsed_data.find(ESP_BLE_AD_TYPE_SERVICE_DATA);
            if (it != parsed_data.end() && it->second.size() == 3)
            {
                return it->second;
            }
            return std::nullopt;
        }

        //ESP_BLE_AD_TYPE_PUBLIC_TARGET
        //ESP_BLE_AD_TYPE_RANDOM_TARGET
        //ESP_BLE_AD_TYPE_LE_DEV_ADDR
        //ESP_BLE_AD_TYPE_SPAIR_C256
        //ESP_BLE_AD_TYPE_SPAIR_R256
        //ESP_BLE_AD_TYPE_32SOL_SRV_UUID
        //ESP_BLE_AD_TYPE_32SERVICE_DATA
        //ESP_BLE_AD_TYPE_128SERVICE_DATA
        //ESP_BLE_AD_TYPE_LE_SECURE_CONFIRM
        //ESP_BLE_AD_TYPE_LE_SECURE_RANDOM
        //ESP_BLE_AD_TYPE_URI
        //ESP_BLE_AD_TYPE_INDOOR_POSITION
        //ESP_BLE_AD_TYPE_TRANS_DISC_DATA
        //ESP_BLE_AD_TYPE_LE_SUPPORT_FEATURE
        //ESP_BLE_AD_TYPE_CHAN_MAP_UPDATE

        #pragma endregion
    };

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
