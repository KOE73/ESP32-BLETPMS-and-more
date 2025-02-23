#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_event.h"
#include "esp_log.h"

static const char* TAG = "EVENT_EXAMPLE";

// Определяем имена "очередей" (event bases)
ESP_EVENT_DEFINE_BASE(SENSOR_EVENTS);  // "Очередь" для сенсоров
ESP_EVENT_DEFINE_BASE(NETWORK_EVENTS); // "Очередь" для сети

// Определяем ID событий (имена внутри очередей)
enum {
    SENSOR_TEMP_UPDATED,
    SENSOR_HUMIDITY_UPDATED
};

enum {
    NETWORK_CONNECTED,
    NETWORK_DISCONNECTED
};

// Обработчик для сенсоров
void sensor_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    if (event_base == SENSOR_EVENTS) {
        switch (event_id) {
            case SENSOR_TEMP_UPDATED:
                ESP_LOGI(TAG, "Temperature updated: %d", *(int*)event_data);
                break;
            case SENSOR_HUMIDITY_UPDATED:
                ESP_LOGI(TAG, "Humidity updated: %d", *(int*)event_data);
                break;
        }
    }
}

// Обработчик для сети
void network_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    if (event_base == NETWORK_EVENTS) {
        switch (event_id) {
            case NETWORK_CONNECTED:
                ESP_LOGI(TAG, "Network connected");
                break;
            case NETWORK_DISCONNECTED:
                ESP_LOGI(TAG, "Network disconnected");
                break;
        }
    }
}

// Задача-отправитель для сенсоров
void sensor_task(void* pvParameters) {
    int temp = 25;
    int humidity = 60;
    while (1) {
        esp_event_post(SENSOR_EVENTS, SENSOR_TEMP_UPDATED, &temp, sizeof(temp), portMAX_DELAY);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
        esp_event_post(SENSOR_EVENTS, SENSOR_HUMIDITY_UPDATED, &humidity, sizeof(humidity), portMAX_DELAY);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}

// Задача-отправитель для сети
void network_task(void* pvParameters) {
    while (1) {
        esp_event_post(NETWORK_EVENTS, NETWORK_CONNECTED, NULL, 0, portMAX_DELAY);
        vTaskDelay(3000 / portTICK_PERIOD_MS);
        esp_event_post(NETWORK_EVENTS, NETWORK_DISCONNECTED, NULL, 0, portMAX_DELAY);
        vTaskDelay(3000 / portTICK_PERIOD_MS);
    }
}

void app_main() {
    // Используем стандартный событийный цикл
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // Подписываем обработчики по именам "очередей"
    ESP_ERROR_CHECK(esp_event_handler_register(SENSOR_EVENTS, ESP_EVENT_ANY_ID, sensor_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(NETWORK_EVENTS, ESP_EVENT_ANY_ID, network_handler, NULL));

    // Запускаем задачи-отправители
    xTaskCreate(sensor_task, "sensor_task", 2048, NULL, 5, NULL);
    xTaskCreate(network_task, "network_task", 2048, NULL, 5, NULL);
}