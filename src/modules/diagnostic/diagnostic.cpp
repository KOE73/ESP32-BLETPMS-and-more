#include "diagnostic.hpp"
#include <stdio.h>
#include <string.h>
#include <esp_log.h>
#include <esp_timer.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "ArduinoJson.h"

// Тег для логов
static const char *TAG = "DIAGNOSTICS";

// Размер буфера для статистики
#define STATS_BUFFER_SIZE 1024

// Определение мьютекса для критической секции
static portMUX_TYPE diagnostics_mux = portMUX_INITIALIZER_UNLOCKED;

// Функция для вывода диагностики
void print_task_diagnostics(void)
{
    char stats_buffer[STATS_BUFFER_SIZE];

    // Защита от прерываний для vTaskList
    taskENTER_CRITICAL(&diagnostics_mux);
    vTaskList(stats_buffer);
    taskEXIT_CRITICAL(&diagnostics_mux);

    // Вывод списка задач
    ESP_LOGI(TAG, "=== Task List ===\nName\t\tState\tPrio\tStack\tNum\n%s", stats_buffer);

    // Защита от прерываний для vTaskGetRunTimeStats
    taskENTER_CRITICAL(&diagnostics_mux);
    vTaskGetRunTimeStats(stats_buffer);
    taskEXIT_CRITICAL(&diagnostics_mux);

    // Вывод статистики времени выполнения
    ESP_LOGI(TAG, "=== Run Time Stats ===\nName\t\tTime\t\t%%\n%s", stats_buffer);

    // Свободная память
    ESP_LOGI(TAG, "Free heap: %u bytes", heap_caps_get_free_size(MALLOC_CAP_8BIT));
    ESP_LOGI(TAG, "=================");
}

// Сопоставление состояния задачи с символом
static char state_to_char(eTaskState state)
{
    switch (state)
    {
    case eRunning:
        return 'R';
    case eReady:
        return 'X';
    case eBlocked:
        return 'B';
    case eSuspended:
        return 'S';
    case eDeleted:
        return 'D';
    default:
        return '?';
    }
}
/*

{
  "msgType": "diagnostic",
  "tasks": [
    {
      "name": "LambdaTask",
      "state": "R",
      "priority": 5,
      "stack_free": 1964,
      "task_num": 15,
      "runtime": 26816,
      "percentage": 0
    }
  ],
  "total_tasks": 15,
  "free_heap": 8506095
}

*/
std::string print_task_diagnostics_json()
{
    UBaseType_t task_count = uxTaskGetNumberOfTasks();

    TaskStatus_t *task_status = (TaskStatus_t *)malloc(task_count * sizeof(TaskStatus_t));
    if (!task_status)
    {
        ESP_LOGE(TAG, "Не удалось выделить память для статусов задач");
        return "No mem";
    }

    // Сбор данных о задачах
    uint32_t total_runtime;
    taskENTER_CRITICAL(&diagnostics_mux);
    UBaseType_t actual_task_count = uxTaskGetSystemState(task_status, task_count, &total_runtime);
    taskEXIT_CRITICAL(&diagnostics_mux);

    /* For percentage calculations. */
    total_runtime /= 100UL;

    JsonDocument doc;
    doc["msgType"] = "diagnostic";

    // Заполнение JSON
    JsonArray tasks_array = doc["tasks"].to<JsonArray>();
    for (UBaseType_t i = 0; i < actual_task_count; i++)
    {
        uint32_t ulStatsAsPercentage = total_runtime > 0 ? task_status[i].ulRunTimeCounter / total_runtime : 0;

        JsonObject task = tasks_array.add<JsonObject>();
        task["name"] = task_status[i].pcTaskName;
        char state_str[2] = {state_to_char(task_status[i].eCurrentState), '\0'};
        task["s"] = state_str;
        task["p"] = task_status[i].uxCurrentPriority;
        task["sf"] = task_status[i].usStackHighWaterMark;
        task["tn"] = task_status[i].xTaskNumber;
        task["rt"] = task_status[i].ulRunTimeCounter;
        task["rp"] = ulStatsAsPercentage;
    }
    doc["total_tasks"] = actual_task_count;
    doc["free_heap"] = heap_caps_get_free_size(MALLOC_CAP_8BIT);

    std::string output;
    serializeJson(doc, output);

    free(task_status);

    return output;
}

// Задача для периодической диагностики
static void diagnostics_task(void *param)
{
    while (1)
    {
        print_task_diagnostics();

        vTaskDelay(pdMS_TO_TICKS(30000)); // Вывод каждые 30 секунд
    }
}

// Инициализация диагностики
void init_diagnostics(void)
{
    // Создание задачи диагностики
    xTaskCreate(
        diagnostics_task, // Функция задачи
        "Diagnostics",    // Имя задачи
        4096,             // Размер стека (в байтах)
        NULL,             // Параметры
        1,                // Приоритет (низкий)
        NULL              // Handle задачи
    );
}