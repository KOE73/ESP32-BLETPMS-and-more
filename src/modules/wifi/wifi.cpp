#include <stdio.h>
#include <string.h>
#include "esp_system.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_event.h"
//#include "esp_http_server.h"


// Теги для логирования
static const char *TAG_WIFI = "WIFI";

// Настройки Wi-Fi
#define WIFI_SSID "Your_SSID"
#define WIFI_PASS "Your_PASSWORD"
#define AP_SSID "ESP32_AP"
#define AP_PASS "12345678"

static void wifi_event_handler(void *ctx, esp_event_base_t event_base,
                               int32_t event_id, void *event_data);


// ====== WIFI FUNCTIONS ======
void wifi_init(void)
{
    // Создание стандартного цикла событий
    // TODO вынести в главный инициализатор
    esp_err_t ret = esp_event_loop_create_default();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG_WIFI, "Failed to create event loop");
        return;
    }

    // Регистрация обработчика событий Wi-Fi и IP
    // Experiment
    esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_CONNECTED, wifi_event_handler, NULL);
    esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, wifi_event_handler, NULL);


    ESP_LOGI(TAG_WIFI, "Initializing Wi-Fi...");
    esp_netif_init();

    esp_netif_create_default_wifi_ap();
    esp_netif_create_default_wifi_sta();
    //esp_netif_create_default_wifi_nan();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);

    wifi_config_t ap_config = {
        .ap = {
            .ssid = AP_SSID,
            .password = AP_PASS,
            .authmode = WIFI_AUTH_WPA2_PSK,
            .max_connection = 4},
    };

    wifi_config_t sta_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
        },
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &ap_config));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &sta_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG_WIFI, "Wi-Fi initialized. AP SSID: %s", AP_SSID);
}

static const char *TAG_WIFI_EVENT = "wifi_event";

// Обработчик события для Wi-Fi
// Experiment
static void wifi_event_handler(void *ctx, esp_event_base_t event_base,
                               int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_CONNECTED)
    {
        ESP_LOGI(TAG_WIFI_EVENT, "Connected to Wi-Fi network");
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ESP_LOGI(TAG_WIFI_EVENT, "Got IP address");
    }
}