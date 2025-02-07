#include <stdio.h>
#include <string.h>
#include "esp_system.h"
#include "esp_log.h"
#include "esp_event.h"
#include <nvs_flash.h>
#include <nvs.h>
#include <nvs_handle.hpp>

#include <modules/ble/ble.hpp>
#include <modules/wifi/wifi.hpp>
#include <modules/web/web.hpp>
#include <modules/store/store.hpp>

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

// ====== MAIN ======
void init_main(void)
{
    ESP_LOGI(TAG_MAIN, "Initializing ESP32...");

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
