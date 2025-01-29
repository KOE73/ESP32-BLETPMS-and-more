// https://github.com/espressif/esp-idf/blob/master/examples/bluetooth/bluedroid/ble_50/periodic_sync/main/periodic_sync_demo.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "nvs_flash.h"

#include "esp_system.h"
#include "esp_log.h"
#include "esp_event.h"

#include "modules/composition_root.hpp"

#include "esp_partition.h"

#include "ble/esp_gap_ble_api_to_string.h"

static const char *TAG_MAIN = "MAIN";
 
void print_partition_table()
{
    const esp_partition_t *partition = NULL;
    esp_partition_iterator_t iter = esp_partition_find(ESP_PARTITION_TYPE_ANY, ESP_PARTITION_SUBTYPE_ANY, NULL);

    while (iter != NULL)
    {
        partition = esp_partition_get(iter);
        printf("Partition name: %s\n", partition->label);
        printf("Type: %d, Subtype: %d\n", partition->type, partition->subtype);
        printf("Offset: 0x%08lX, Size: 0x%08lX\n", partition->address, partition->size);
        iter = esp_partition_next(iter);
    }
}

extern "C" void app_main()
{

     vTaskDelay(pdMS_TO_TICKS(4000)); // Задержка на 1000 миллисекунд
    ESP_LOGI(TAG_MAIN, "Программа запущена V3 !");

    for (int i = 0; i < 10; i++)
    {
        vTaskDelay(pdMS_TO_TICKS(250)); // Задержка на 1000 миллисекунд
        ESP_LOGI(TAG_MAIN, "%d", i);
    }

    print_partition_table();

    // Создание стандартного цикла событий
    // TODO вынести в главный инициализатор
    esp_err_t ret = esp_event_loop_create_default();
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG_MAIN, "Failed to create event loop");
        return;
    }


    init_main();

    vTaskDelay(200 / portTICK_PERIOD_MS);

    while (1)
    {
        printf("\033[03;38;05;222mMain loop running.\033[0m\n");
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}