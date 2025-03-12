

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
    
    class BtDeviceRecognizerTest : public BtDeviceRecognizerBase
    {
    public:
        const char *getName() override
        {
            return "BtDeviceRecognizerTest";
        }

        bool CanHandle(const BleGapExtAdvReport &report) override;
        void Log(const BleGapExtAdvReport &report) override;
         void SendEvent(esp_event_loop_handle_t yabt_loop,const BleGapExtAdvReport &report)override ;

    private:
    BtDeviceRecognizerTest() = default; // Конструктор теперь пустой

        static BtDeviceRecognizerTest instance; // Статический экземпляр

    public:
        TPMSData parseTPMSData(const std::span<const uint8_t> &rawData);
    };

} // namespace yabt
