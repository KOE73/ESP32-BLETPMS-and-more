
Для работы с расширенным сканированием BLE (Extended Scanning) в ESP-IDF, поддерживающим возможности BLE 5.0 и выше, используются специальные события и константы, определённые в API GAP (esp_gap_ble_api.h). Расширенное сканирование позволяет обрабатывать новые типы рекламы, такие как Extended Advertising и Periodic Advertising, что обеспечивает большую гибкость в настройке и приёме данных. Ниже описаны основные события, константы и пример реализации для расширенного сканирования.

----------

### События расширенного сканирования BLE

События для расширенного сканирования определены в перечислении esp_gap_ble_cb_event_t:

1.  **ESP_GAP_BLE_EXT_SCAN_PARAM_SET_COMPLETE_EVT**
    -   **Описание**: Завершение установки параметров расширенного сканирования (функция esp_ble_gap_ext_scan_set_params).
    -   **Параметры** (esp_ble_gap_cb_param_t.ext_scan_param_set_cmpl):  
        -   status: Статус операции (ESP_BT_STATUS_SUCCESS при успехе).
  
3.  **ESP_GAP_BLE_EXT_SCAN_START_COMPLETE_EVT**
    -   **Описание**: Завершение запуска расширенного сканирования (функция esp_ble_gap_start_ext_scan).
      
    -   **Параметры** (esp_ble_gap_cb_param_t.ext_scan_start_cmpl):  
        -   status: Статус операции.
  
5.  **ESP_GAP_BLE_EXT_ADV_REPORT_EVT**
    -   **Описание**: Получение расширенного рекламного отчёта.
      
    -   **Параметры** (esp_ble_gap_cb_param_t.ext_adv_report):  
        -   addr_type: Тип адреса устройства.
        -   addr: Адрес устройства (esp_bd_addr_t).
        -   primary_phy: Первичный PHY (например, ESP_BLE_GAP_PHY_1M).
        -   secondary_phy: Вторичный PHY (например, ESP_BLE_GAP_PHY_CODED).
        -   sid: Идентификатор набора рекламы (Advertising Set ID).
        -   tx_power: Мощность передачи.
        -   rssi: RSSI.
        -   peri_adv_interval: Интервал периодической рекламы.
        -   direct_addr: Адрес, если реклама направленная.
        -   adv_type: Тип рекламы (например, ESP_BLE_GAP_SET_EXT_ADV_PROP_CONNECTABLE).
        -   data_status: Статус данных (полные или частичные).
        -   adv_data_len: Длина данных.
        -   adv_data: Данные рекламы (массив байтов).
  
7.  **ESP_GAP_BLE_PERIODIC_ADV_REPORT_EVT**
    -   **Описание**: Получение отчёта о периодической рекламе.
    -   **Параметры** (esp_ble_gap_cb_param_t.periodic_adv_report):  
        -   sync_handle: Дескриптор синхронизации.
        -   tx_power: Мощность передачи.
        -   rssi: RSSI.
        -   data_status: Статус данных.
        -   data_len: Длина данных.
        -   data: Данные периодической рекламы.
  
9.  **ESP_GAP_BLE_EXT_SCAN_STOP_COMPLETE_EVT**
    -   **Описание**: Завершение остановки расширенного сканирования (функция esp_ble_gap_stop_ext_scan).
      
    -   **Параметры** (esp_ble_gap_cb_param_t.ext_scan_stop_cmpl):  
        -   status: Статус операции.
  
11.  **ESP_GAP_BLE_PERIODIC_ADV_SYNC_ESTAB_EVT**
    -   **Описание**: Установление синхронизации с периодической рекламой.
    -   **Параметры** (esp_ble_gap_cb_param_t.periodic_adv_sync_estab):  
        -   status: Статус синхронизации.
        -   sync_handle: Дескриптор синхронизации.
        -   sid: Идентификатор набора.
        -   adv_addr: Адрес устройства.
        -   adv_phy: PHY рекламы.
        -   peri_adv_interval: Интервал рекламы.
        -   adv_clk_accuracy: Точность часов.
  
13.  **ESP_GAP_BLE_PERIODIC_ADV_CREATE_SYNC_CANCEL_EVT**
    -   **Описание**: Отмена создания синхронизации с периодической рекламой.
    -   **Параметры** (esp_ble_gap_cb_param_t.periodic_adv_create_sync_cancel):  
        -   status: Статус операции.
          
----------

### Константы расширенного сканирования BLE

Константы используются для настройки и обработки событий расширенного сканирования.

#### Типы PHY (esp_ble_gap_phy_t)
-   ESP_BLE_GAP_PHY_1M: PHY 1M (стандартный для BLE 4.x).
-   ESP_BLE_GAP_PHY_2M: PHY 2M (BLE 5.0).
-   ESP_BLE_GAP_PHY_CODED: Кодированный PHY для увеличения дальности (BLE 5.0).

#### Типы адресов (esp_ble_addr_type_t)
-   BLE_ADDR_TYPE_PUBLIC: Публичный адрес.
-   BLE_ADDR_TYPE_RANDOM: Случайный адрес.
-   Другие типы, аналогичные классическому сканированию.

#### Флаги фильтрации (esp_ble_scan_filter_t)
-   BLE_SCAN_FILTER_ALLOW_ALL: Разрешить все устройства.
-   Другие флаги, аналогичные классическому сканированию.

#### Свойства расширенной рекламы (esp_ble_gap_set_ext_adv_prop_t)
-   ESP_BLE_GAP_SET_EXT_ADV_PROP_CONNECTABLE: Подключаемая реклама.
-   ESP_BLE_GAP_SET_EXT_ADV_PROP_SCANNABLE: Сканируемая реклама.
-   ESP_BLE_GAP_SET_EXT_ADV_PROP_DIRECTED: Направленная реклама.
-   ESP_BLE_GAP_SET_EXT_ADV_PROP_NONCONN: Неподключаемая реклама.
-   ESP_BLE_GAP_SET_EXT_ADV_PROP_ANON: Анонимная реклама.
-   ESP_BLE_GAP_SET_EXT_ADV_PROP_TX_PWR: Включение информации о мощности.

#### Статусы данных (esp_ble_gap_data_status_t)
-   ESP_BLE_GAP_DATA_STATUS_COMPLETE: Полные данные.
-   ESP_BLE_GAP_DATA_STATUS_INCOMPLETE: Неполные данные.
-   ESP_BLE_GAP_DATA_STATUS_TRUNCATED: Обрезанные данные.

#### Маски конфигурации (esp_ble_gap_ext_scan_cfg_mask_t)
-   ESP_BLE_GAP_EXT_SCAN_CFG_UNCODE_MASK: Конфигурация для некодированного PHY.
-   ESP_BLE_GAP_EXT_SCAN_CFG_CODE_MASK: Конфигурация для кодированного PHY.

#### Типы сканирования (esp_ble_scan_type_t)
-   Аналогично классическому сканированию.

----------

### Пример расширенного сканирования BLE

Пример кода демонстрирует базовую реализацию расширенного сканирования:

``` C++

#include "esp_log.h"
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_gap_ble_api.h"

static const char* TAG = "EXT_BLE_SCAN";

static void gap_cb(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t* param) {
    switch (event) {
        case ESP_GAP_BLE_EXT_SCAN_PARAM_SET_COMPLETE_EVT:
            if (param->ext_scan_param_set_cmpl.status == ESP_BT_STATUS_SUCCESS) {
                ESP_LOGI(TAG, "Extended scan params set, starting scan...");
                esp_ble_gap_start_ext_scan(0, 0); // Бесконечное сканирование
            }
            break;
        case ESP_GAP_BLE_EXT_SCAN_START_COMPLETE_EVT:
            if (param->ext_scan_start_cmpl.status == ESP_BT_STATUS_SUCCESS) {
                ESP_LOGI(TAG, "Extended scan started");
            }
            break;
        case ESP_GAP_BLE_EXT_ADV_REPORT_EVT: {
            esp_ble_gap_ext_adv_report_t* report = &param->ext_adv_report;
            ESP_LOGI(TAG, "Extended Adv Report - Addr: %02x:%02x:%02x:%02x:%02x:%02x, RSSI: %d",
                     report->addr.addr[5], report->addr.addr[4], report->addr.addr[3],
                     report->addr.addr[2], report->addr.addr[1], report->addr.addr[0],
                     report->rssi);
            ESP_LOG_BUFFER_HEX(TAG, report->adv_data, report->adv_data_len);
            break;
        }
        case ESP_GAP_BLE_EXT_SCAN_STOP_COMPLETE_EVT:
            if (param->ext_scan_stop_cmpl.status == ESP_BT_STATUS_SUCCESS) {
                ESP_LOGI(TAG, "Extended scan stopped");
            }
            break;
        case ESP_GAP_BLE_PERIODIC_ADV_REPORT_EVT: {
            esp_ble_gap_periodic_adv_report_t* report = &param->periodic_adv_report;
            ESP_LOGI(TAG, "Periodic Adv Report - Sync Handle: %d, RSSI: %d", report->sync_handle, report->rssi);
            ESP_LOG_BUFFER_HEX(TAG, report->data, report->data_len);
            break;
        }
        default:
            break;
    }
}

void ble_init() {
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_bt_controller_init(&bt_cfg));
    ESP_ERROR_CHECK(esp_bt_controller_enable(ESP_BT_MODE_BLE));
    ESP_ERROR_CHECK(esp_bluedroid_init());
    ESP_ERROR_CHECK(esp_bluedroid_enable());
    ESP_ERROR_CHECK(esp_ble_gap_register_callback(gap_cb));

    // Настройка параметров расширенного сканирования
    esp_ble_ext_scan_params_t ext_params = {
        .own_addr_type = BLE_ADDR_TYPE_PUBLIC,
        .filter_policy = BLE_SCAN_FILTER_ALLOW_ALL,
        .scan_duplicate = BLE_SCAN_DUPLICATE_ENABLE,
        .cfg_mask = ESP_BLE_GAP_EXT_SCAN_CFG_UNCODE_MASK | ESP_BLE_GAP_EXT_SCAN_CFG_CODE_MASK,
        .uncoded_cfg = { .interval = 0x50, .window = 0x30 },
        .coded_cfg = { .interval = 0x50, .window = 0x30 }
    };
    ESP_ERROR_CHECK(esp_ble_gap_ext_scan_set_params(ESP_BLE_GAP_SET_EXT_SCAN_PARAM_ALL, &ext_params));
}

void app_main() {
    ESP_ERROR_CHECK(nvs_flash_init());
    ble_init();
}
```

----------

### Итог

Для расширенного сканирования BLE в ESP-IDF используются события, такие как EXT_SCAN_PARAM_SET_COMPLETE, EXT_SCAN_START_COMPLETE, EXT_ADV_REPORT, PERIODIC_ADV_REPORT, EXT_SCAN_STOP_COMPLETE и другие. Константы включают типы PHY, свойства рекламы и статусы данных. Приведённый пример кода демонстрирует базовую реализацию расширенного сканирования, которую можно адаптировать под конкретные задачи.