

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

#include "yabt.hpp"
#include "yabt_samsung_EIT5300_data.hpp"

namespace yabt
{

    class BtDeviceRecognizerSmartTag : public BtDeviceRecognizerBase
    {
    public:
        const char *getName() override
        {
            return "BtDeviceRecognizerSmartTag";
        }

        bool CanHandle(const BleGapExtAdvReport &report) override;

        void Log(const BleGapExtAdvReport &report) override;

        void SendEvent(esp_event_loop_handle_t yabt_loop,
                       const BleGapExtAdvReport &report) override;

    protected:
        BtDeviceRecognizerSmartTag() = default;
        BtDeviceRecognizerSmartTag(SkipRegister skip)
            : BtDeviceRecognizerBase(skip)
        {
        }

        static BtDeviceRecognizerSmartTag instance; // статический экземпляр

        virtual bool parseSmartTagData(const BleGapExtAdvReport &report, SmartTagFD5AData &data);
    };

    // Возможная специализация для известных устройств SmartTag (по MAC)
    class BtDeviceRecognizerSmartTagWithAddress : public BtDeviceRecognizerSmartTag, public BtKnownDevice
    {
    public:
        BtDeviceRecognizerSmartTagWithAddress(esp_bd_addr_t *esp_addr)
            : BtDeviceRecognizerSmartTag(SkipRegister{}), BtKnownDevice(esp_addr)
        {
        }

        bool parseSmartTagData(const BleGapExtAdvReport &report, SmartTagFD5AData &data) override;
    };
} // namespace yabt
