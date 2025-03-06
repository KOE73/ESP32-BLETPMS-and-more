

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
