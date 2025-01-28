#include <stdio.h>
#include <string.h>
#include "esp_system.h"
#include "esp_log.h"
//#include "nvs_flash.h"
#include "esp_event.h"

#include <modules/ble/ble.hpp>
#include <modules/wifi/web.hpp>
#include <modules/web/web.hpp>
#include <modules/store/store.hpp>

// pio run -t menuconfig

// Теги для логирования
static const char *TAG_MAIN = "MAIN";

// Прототипы функций
//void wifi_init(void);
//void ble_init(void);
//void start_ble_scan(void);
//esp_err_t start_web_server(void);
//void fs_init(void);

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
    start_ble_scan();


}
