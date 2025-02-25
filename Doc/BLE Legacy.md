### События классического сканирования BLE

События определены в перечислении esp_gap_ble_cb_event_t в esp_gap_ble_api.h. Вот те, что связаны с классическим сканированием:

1.  **ESP_GAP_BLE_SCAN_PARAM_SET_COMPLETE_EVT**
    -   **Описание**: Завершение установки параметров сканирования (esp_ble_gap_set_scan_params).
      
    -   **Параметры**: esp_ble_gap_cb_param_t.scan_param_cmpl:  
        -   status: Статус операции (ESP_BT_STATUS_SUCCESS при успехе).
  
3.  **ESP_GAP_BLE_SCAN_START_COMPLETE_EVT**
    -   **Описание**: Завершение запуска сканирования (esp_ble_gap_start_scanning).
      
    -   **Параметры**: esp_ble_gap_cb_param_t.scan_start_cmpl:  
        -   status: Статус операции.
  
5.  **ESP_GAP_BLE_SCAN_RESULT_EVT**
    -   **Описание**: Получение результата сканирования (рекламного пакета от устройства).
      
    -   **Параметры**: esp_ble_gap_cb_param_t.scan_rst:  
        -   bda: Адрес устройства (esp_bd_addr_t).
        -   dev_type: Тип устройства (ESP_BT_DEVICE_TYPE_BLE для BLE).
        -   ble_addr_type: Тип адреса (BLE_ADDR_TYPE_PUBLIC, BLE_ADDR_TYPE_RANDOM, и т.д.).
        -   ble_evt_type: Тип события (например, ESP_BLE_EVT_NON_CONN_IND, ESP_BLE_EVT_CONN_ADV).
        -   rssi: Уровень сигнала (RSSI, в дБм).
        -   ble_adv: Данные рекламы (массив байтов).
        -   adv_data_len: Длина данных рекламы.
        -   flag: Флаги рекламы (например, ESP_BLE_AD_FLAG_LIMITED, ESP_BLE_AD_FLAG_GEN_DISC).
        -   num_resps: Количество ответов.
        -   search_evt: Тип результата (например, ESP_GAP_SEARCH_INQ_RES_EVT).
  
7.  **ESP_GAP_BLE_SCAN_STOP_COMPLETE_EVT**
    -   **Описание**: Завершение остановки сканирования (esp_ble_gap_stop_scanning).
      
    -   **Параметры**: esp_ble_gap_cb_param_t.scan_stop_cmpl:  
        -   status: Статус операции.

9.  **ESP_GAP_BLE_UPDATE_CONN_PARAMS_EVT**
    -   **Описание**: Обновление параметров соединения (может быть связано с подключением после сканирования).
      
    -   **Параметры**: esp_ble_gap_cb_param_t.update_conn_params:  
        -   status, conn_int, latency, timeout, и т.д.
          
----------

### Константы классического сканирования BLE

Эти константы используются для настройки и обработки событий классического сканирования.

#### Типы сканирования (esp_ble_scan_type_t)

-   **BLE_SCAN_TYPE_PASSIVE**: Пассивное сканирование (только приём рекламы).
-   **BLE_SCAN_TYPE_ACTIVE**: Активное сканирование (запрос дополнительных данных через Scan Request).

#### Типы адресов (esp_ble_addr_type_t)

-   **BLE_ADDR_TYPE_PUBLIC**: Публичный адрес.
-   **BLE_ADDR_TYPE_RANDOM**: Случайный адрес.
-   **BLE_ADDR_TYPE_RPA_PUBLIC**: RPA (Resolvable Private Address) с публичным ID.
-   **BLE_ADDR_TYPE_RPA_RANDOM**: RPA со случайным ID.

#### Фильтры сканирования (esp_ble_scan_filter_t)

-   **BLE_SCAN_FILTER_ALLOW_ALL**: Принимать все устройства.
-   **BLE_SCAN_FILTER_ALLOW_ONLY_WLST**: Только устройства из белого списка.
-   **BLE_SCAN_FILTER_ALLOW_UND_RPA_DIR**: Устройства с неразрешённым RPA и направленной рекламой.
-   **BLE_SCAN_FILTER_ALLOW_WLIST_RPA_DIR**: Устройства из белого списка с RPA и направленной рекламой.

#### Типы событий рекламы (esp_ble_evt_type_t)
-   **ESP_BLE_EVT_CONN_ADV**: Подключаемая ненаправленная реклама.
-   **ESP_BLE_EVT_CONN_DIR**: Подключаемая направленная реклама.
-   **ESP_BLE_EVT_DISC_ADV**: Обнаруживаемая ненаправленная реклама.
-   **ESP_BLE_EVT_NON_CONN_IND**: Неподключаемая ненаправленная реклама.
-   **ESP_BLE_EVT_SCAN_RSP**: Ответ на запрос сканирования.

#### Флаги рекламы (esp_ble_advertise_flag_t)
-   **ESP_BLE_AD_FLAG_LIMITED**: Ограниченное обнаружение.
-   **ESP_BLE_AD_FLAG_GEN_DISC**: Общее обнаружение.
-   **ESP_BLE_AD_FLAG_BREDR_NOT_SPT**: Не поддерживает BR/EDR.
-   **ESP_BLE_AD_FLAG_DMT_CONTROLLER_SPT**: Поддержка контроллера Dual Mode.
-   **ESP_BLE_AD_FLAG_DMT_HOST_SPT**: Поддержка хоста Dual Mode.

#### Результаты поиска (esp_gap_search_event_t)
-   **ESP_GAP_SEARCH_INQ_RES_EVT**: Результат сканирования (найдено устройство).
-   **ESP_GAP_SEARCH_INQ_CMPL_EVT**: Завершение сканирования.
-   **ESP_GAP_SEARCH_DISC_RES_EVT**: Результат обнаружения классического Bluetooth.
-   **ESP_GAP_SEARCH_DISC_CMPL_EVT**: Завершение обнаружения классического Bluetooth.
-   **ESP_GAP_SEARCH_SEARCH_CANCEL_CMPL_EVT**: Отмена сканирования завершена.

#### Статусы операций (esp_bt_status_t)
-   **ESP_BT_STATUS_SUCCESS**: Успех.
-   **ESP_BT_STATUS_FAIL**: Общая ошибка.
-   **ESP_BT_STATUS_NOT_READY**: Не готово.
-   **ESP_BT_STATUS_NOMEM**: Нет памяти.
-   **ESP_BT_STATUS_BUSY**: Занято.