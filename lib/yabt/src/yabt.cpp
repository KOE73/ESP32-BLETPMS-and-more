

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
#include "esp_event.h"

#include "yabt.hpp"

#define TAG_BTController "BTController"

// const char *YABT_EVENT = "YABT_EVENT";
ESP_EVENT_DEFINE_BASE(YABT_EVENT);


__attribute__((weak)) esp_event_loop_handle_t CBOR_loop = NULL;
__attribute__((weak)) esp_event_base_t CBOR_EVENT = NULL;

namespace yabt
{

    esp_event_loop_handle_t BTController::event_loop_;

    BTController::BTController()
    {
        // Используйте uxTaskGetStackHighWaterMark для мониторинга использования стека и корректировки его размера, если это необходимо.

        esp_event_loop_args_t loop_args = {
            .queue_size = 10,         // Размер очереди событий
            .task_name = "yabt_loop", // Имя задачи
            .task_priority = 5,       // Приоритет задачи
            .task_stack_size = 6000,  // Размер стека
            .task_core_id = 0         // Ядро процессора (0 или 1)
        };

        // Создание пользовательского цикла событий
        ESP_ERROR_CHECK(esp_event_loop_create(&loop_args, &event_loop_));
    };

    bool BTController::GapHanler(const BleGapExtAdvReport report)
    {

        // 1. обработка по известным адресам
        auto known_handler_iter = known_addreses_gap_Handlers_.find(report.getAddr());
        if (known_handler_iter != known_addreses_gap_Handlers_.end())
        {
            BtDeviceRecognizerBase *known_device_handler = known_handler_iter->second;
    
            if (known_device_handler->CanHandle(report))
            {
                known_device_handler->Log(report);
                known_device_handler->SendEvent(event_loop_, report);
                return true;
            }
        }
        
        // 2. Перебираем все распознаватели
        if (search_mode_)
        {
            for (auto &recognizer : gap_recognizers_)
            {
                if (recognizer->CanHandle(report))
                {
                    recognizer->Log(report);
                    recognizer->SendEvent(event_loop_, report);

                    return true; // Завершаем обработку
                }
            }
        }

        return false; // Если ни один распознаватель не вернул true
    }

} // namespace yabt
