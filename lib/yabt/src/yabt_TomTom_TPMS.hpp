

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
#include "yabt_tpms.hpp"



namespace yabt
{

    class BtDeviceRecognizerTomTomTPMS : public BtDeviceRecognizerBase
    {
    public:
        const char *getName() override
        {
            return "BtDeviceRecognizerTomTomTPMS";
        }

        bool CanHandle(const BleGapExtAdvReport &report) override;
        
        void Log(const BleGapExtAdvReport &report) override;

        void SendEvent(esp_event_loop_handle_t yabt_loop, const BleGapExtAdvReport &report) override;

    protected:
        BtDeviceRecognizerTomTomTPMS() = default;
        BtDeviceRecognizerTomTomTPMS(SkipRegister skip):BtDeviceRecognizerBase(skip) {}

        static BtDeviceRecognizerTomTomTPMS instance; // Статический экземпляр

       virtual void parseTPMSData(const BleGapExtAdvReport &report, TPMSData &data);
    };

    class BtDeviceRecognizerTomTomTPMSWithAddress : public BtDeviceRecognizerTomTomTPMS, public BtKnownDevice
    {
    public:
        BtDeviceRecognizerTomTomTPMSWithAddress(esp_bd_addr_t *esp_addr) :BtDeviceRecognizerTomTomTPMS(SkipRegister{}), BtKnownDevice(esp_addr)
        {
        }

        void parseTPMSData(const BleGapExtAdvReport &report, TPMSData &data)override;

    };

} // namespace yabt
