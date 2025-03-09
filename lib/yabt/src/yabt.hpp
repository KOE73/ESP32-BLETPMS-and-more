#pragma once

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
#include "esp_log.h"
// #include "esp_event.h"
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_gap_ble_api.h"
// #include "esp_event_loop.h"

#include "yabt_utils.hpp"
#include "yabt_events.hpp"

#define TAG_BTController "BTController"

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
    class GapHandlerBase;

    // Фабрика. Пока думаю зачем
    class Factory
    {
    public:
        using CreateFunc = std::unique_ptr<GapHandlerBase> (*)();

        static Factory &instance()
        {
            static Factory f;
            return f;
        }

        void register_class(const std::string &name, CreateFunc func)
        {
            creators[name] = func;
        }

        std::vector<std::unique_ptr<GapHandlerBase>> create_all()
        {
            std::vector<std::unique_ptr<GapHandlerBase>> objects;
            for (const auto &[name, func] : creators)
            {
                objects.push_back(func());
            }
            return objects;
        }

    private:
        std::unordered_map<std::string, CreateFunc> creators;
    };

    class BtDeviceRecognizerBase;
    class BTController
    {
    private:
        std::vector<BtDeviceRecognizerBase *> recognizers_;
        std::map<BtDeviceAddr, GapHandlerBase> known_addreses_Handlers_;
        static esp_event_loop_handle_t event_loop_;

    public:
        explicit BTController();
        BTController(const BTController &) = delete;
        BTController &operator=(const BTController &) = delete;
        BTController(BTController &&) = delete;
        BTController &operator=(BTController &&) = delete;

        static BTController &getInstance()
        {
            static BTController instance; // Создаётся один раз и хранится в памяти
            return instance;
        }

        esp_event_loop_handle_t getEventLoop() const { return event_loop_; }

        void AddBtDeviceRecognizer(BtDeviceRecognizerBase *recognizer)
        {
            recognizers_.push_back(recognizer);
        }

        bool GapHanler(BleGapExtAdvReport report);
    };

    class BtDeviceRecognizerBase
    {
    public:
        virtual ~BtDeviceRecognizerBase() = default;
        virtual const char *getName() = 0;
        virtual bool GapHandler(const BleGapExtAdvReport &report) = 0;
        virtual void Log(const BleGapExtAdvReport &report)
        {
            ESP_LOGI("BtDeviceRecognizerBase", "%s", getName());
        };
        virtual void SendEvent(esp_event_loop_handle_t yabt_loop, const BleGapExtAdvReport &report) {};

    protected:
        BtDeviceRecognizerBase()
        {
            BTController::getInstance().AddBtDeviceRecognizer(this);
        }
    };

    /// @brief Список обработчиков с известными адресами

    class GapHandlerBase
    {
    private:
        /* data */
    public:
        GapHandlerBase(/* args */) {};
        ~GapHandlerBase() {};

        void handle(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param);
    };

    class bhandler_gap_addr
    {
    protected:
        BtDeviceAddr addr_;

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
        };
    };

} // namespace yabt
