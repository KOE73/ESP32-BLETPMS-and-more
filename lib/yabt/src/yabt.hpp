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

// Для фабрики. Пока думаю заче
// #define REGISTER_CLASS(Derived)
//     namespace
//     {
//         struct Register##Derived
//         {
//             Register##Derived()
//             {
//                 Factory::instance().register_class(#Derived, []() -> std::unique_ptr<Base> { return std::make_unique<Derived>(); });
//             }
//         } reg##Derived;
//

namespace yabt
{
    class GapHandlerBase;

    // // Фабрика. Пока думаю зачем
    // class Factory
    // {
    // public:
    //     using CreateFunc = std::unique_ptr<GapHandlerBase> (*)();
    //
    //     static Factory &instance()
    //     {
    //         static Factory f;
    //         return f;
    //     }
    //
    //     void register_class(const std::string &name, CreateFunc func)
    //     {
    //         creators[name] = func;
    //     }
    //
    //     std::vector<std::unique_ptr<GapHandlerBase>> create_all()
    //     {
    //         std::vector<std::unique_ptr<GapHandlerBase>> objects;
    //         for (const auto &[name, func] : creators)
    //         {
    //             objects.push_back(func());
    //         }
    //         return objects;
    //     }
    //
    // private:
    //     std::unordered_map<std::string, CreateFunc> creators;
    // };

    class BtDeviceRecognizerBase;

    class BTController
    {
    private:
        std::vector<BtDeviceRecognizerBase *> gap_recognizers_;
        std::map<BtDeviceAddr, BtDeviceRecognizerBase *> known_addreses_gap_Handlers_;
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
            gap_recognizers_.push_back(recognizer);
        }

        bool GapHanler(BleGapExtAdvReport report);
    };

    class BtDeviceRecognizerBase
    {
    public:
        /// @brief  Unique name of the handler.
        ///         Should be based on the manufacturer name and type.
        virtual const char *getName() = 0;

        /// @brief  Determines whether the given record matches
        ///         the detectable device and whether this class
        ///         should continue processing the record.
        /// @param report
        /// @return Returns true if the data is suitable.
        virtual bool CanHandle(const BleGapExtAdvReport &report) = 0;

        virtual void Log(const BleGapExtAdvReport &report)
        {
            ESP_LOGI("BtDeviceRecognizerBase", "%s", getName());
        };

        /// @brief  Sends an event with data to the specified queue.
        ///         The recognizer sends an event without an ID.
        ///         The address handler additionally fills in the identifier.
        /// @param yabt_loop
        /// @param report
        virtual void SendEvent(esp_event_loop_handle_t yabt_loop, const BleGapExtAdvReport &report) {};

    protected:
        /// @brief  Constructor for the unknown device handler.
        ///         Automatically added to BTController.
        ///         Created automatically and exists as a single instance.
        BtDeviceRecognizerBase()
        {
            BTController::getInstance().AddBtDeviceRecognizer(this);
        }

        struct SkipRegister
        {
        };
        /// @brief  Constructor for the known device handler.
        ///         Created when necessary.
        BtDeviceRecognizerBase(SkipRegister) {}
    };

    // class GapHandlerBase
    //{
    // private:
    //     /* data */
    // public:
    //     GapHandlerBase(/* args */) {};
    //     ~GapHandlerBase() {};
    //
    //    void handle(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param);
    //};

    /// @brief  Base class for devices with a known address.
    ///         Unlike a recognizer, it includes states.
    ///         Each address corresponds to its own object,
    ///         allowing for logic to track changes.
    class BtKnownDevice
    {
    protected:
        BtDeviceAddr addr_;

    public:
        BtKnownDevice(esp_bd_addr_t *esp_addr)
        {
            addr_ = esp_addr;
        };

        // virtual ~BtKnownDevice() = default;

        // /// @brief Checking that the request can be processed. Basic by address only.
        // /// @param param
        // /// @return true - if can
        // virtual bool canHandle(esp_ble_gap_ext_adv_report_t *param) { return addr_ == param->addr; }
        //
        // virtual bool handle(esp_ble_gap_ext_adv_report_t *param)
        // {
        //     if (addr_ != param->addr)
        //         return false;
        // }
    };

#define DEVICEDATA_ID_LENGTH 16
#define DEVICEDATA_TYPE_LENGTH 16

    /// @brief  Base data structure for devices.
    struct DeviceData
    {
        /// @brief  Name of the structure type.
        ///         The idea is that similar device types have
        ///         roughly the same data and can populate
        ///         a generalized structure in the same way.
        char type[DEVICEDATA_TYPE_LENGTH];

        /// @brief  String representation of a known device.
        ///         Empty for unknown devices.
        char id[DEVICEDATA_ID_LENGTH];
    };

} // namespace yabt
