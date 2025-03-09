

#include "string.h"

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
#include <optional>

#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_gap_ble_api.h"

#include "yabt_events.hpp"
#include "yabt_Test.hpp"

namespace yabt
{
    // Определение статического экземпляра (создаётся автоматически)
    BtDeviceRecognizerTest BtDeviceRecognizerTest::instance;

    // COOLSPO Address
    static esp_bd_addr_t target_addr = {0xc3, 0x2f, 0x4c, 0xf4, 0xfe, 0x52};

    bool BtDeviceRecognizerTest::GapHandler(const BleGapExtAdvReport &report)
    {
        return report.getAddr() == target_addr;
    }

    void BtDeviceRecognizerTest::Log(const BleGapExtAdvReport &report)
    {
        ESP_LOGI(TAG_BTController, " /////////////// Test : %s", getName());
    }

    void BtDeviceRecognizerTest::SendEvent(esp_event_loop_handle_t yabt_loop, const BleGapExtAdvReport &report)
    {
        TPMSData tpmsData;
        strncpy(tpmsData.manufacturerName, "TESTtest", TPMSDATA_MANUFACTERENAME_LENGTH);

        // Отправка события
        ESP_ERROR_CHECK_WITHOUT_ABORT(esp_event_post_to(
            yabt_loop,
            YABT_EVENT,         // База событий
            YABT_EVENT_TPMS,    // ID события
            &tpmsData,          // Указатель на данные (опционально)
            sizeof(TPMSData),   // Размер данных
            pdMS_TO_TICKS(3000) // ms or portMAX_DELAY     // Тайм-аут (ожидание, если очередь полна)
            ));
    }

} // namespace yabt
