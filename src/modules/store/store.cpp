#include <stdio.h>
#include <string.h>
#include "esp_system.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_spiffs.h"

// Теги для логирования
static const char *TAG_FS = "FS";

// ====== FILE SYSTEM FUNCTIONS ======
void fs_init(void)
{

    // Инициализация NVS
    ESP_ERROR_CHECK(nvs_flash_init());

    ESP_LOGI(TAG_FS, "Initializing SPIFFS...");
    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/spiffs",
        .partition_label = NULL,
        .max_files = 5,
        .format_if_mount_failed = true};

    esp_err_t ret = esp_vfs_spiffs_register(&conf);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG_FS, "Failed to initialize SPIFFS: %s", esp_err_to_name(ret));
        return;
    }

    size_t total = 0, used = 0;
    ret = esp_spiffs_info(NULL, &total, &used);
    if (ret == ESP_OK)
    {
        ESP_LOGI(TAG_FS, "SPIFFS initialized. Total: %d, Used: %d", total, used);
    }
    else
    {
        ESP_LOGE(TAG_FS, "Failed to get SPIFFS info: %s", esp_err_to_name(ret));
    }
}
