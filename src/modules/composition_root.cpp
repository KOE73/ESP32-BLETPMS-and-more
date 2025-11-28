#include <stdio.h>
#include <string.h>
#include "esp_system.h"
#include "esp_log.h"
#include "esp_event.h"
#include <nvs_flash.h>
#include <nvs.h>
#include <nvs_handle.hpp>

#include "modules/ble/ble.hpp"
#include "modules/wifi/wifi.hpp"
#include "modules/web/web.hpp"
#include "modules/store/store.hpp"
#include "modules/diagnostic/diagnostic.hpp"

extern void lcd_main(void);

// pio run -t menuconfig

// Теги для логирования
static const char *TAG_MAIN = "MAIN";

void list_nvs_entries()
{
    esp_err_t err;
    nvs_iterator_t it = NULL;
    nvs_entry_info_t info;

    // Открываем итератор для поиска в пространстве "storage"
    err = nvs_entry_find(NVS_DEFAULT_PART_NAME, "storage", NVS_TYPE_ANY, &it);

    if (it == NULL)
    {
        printf("No NVS entries found.\n");
        return;
    }

    // Перебираем все ключи
    while (it != NULL)
    {
        nvs_entry_info(it, &info); // Получаем информацию о ключе
        printf("Namespace: %s, Key: %s, Type: %d\n", info.namespace_name, info.key, info.type);

        err = nvs_entry_next(&it); // Переход к следующему ключу
    }
}

esp_event_loop_handle_t CBOR_loop = NULL;
esp_event_base_t CBOR_EVENT = "CBOR_EVENT";

// ====== MAIN ======
void init_main(void)
{
    esp_event_loop_args_t loop_args = {
        .queue_size = 20,              // Размер очереди событий
        .task_name = "cbor_loop",      // Имя задачи
        .task_priority = 5,            // Приоритет задачи
        .task_stack_size = 6000,       // Размер стека
        .task_core_id = tskNO_AFFINITY // Ядро процессора (0 или 1)
    };
    
    // CBOR event loop
    ESP_ERROR_CHECK(esp_event_loop_create(&loop_args, &CBOR_loop));
    ESP_LOGI(TAG_MAIN, "CBOR_loop Initialized.");

    ESP_LOGI(TAG_MAIN, "Initializing ESP32...");
    init_diagnostics();
    print_task_diagnostics();

        lcd_main();

    // Инициализация файловой системы
    fs_init();

    // Инициализация Wi-Fi (AP + STA)
    wifi_init();

    // Инициализация BLE
    ble_init();
    // start_ble_scan();

    start_web_server();

    list_nvs_entries();

}
