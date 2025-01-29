#include <stdio.h>
#include <string.h>
#include "esp_system.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_mac.h"
// #include "esp_http_server.h"

#include "modules/wifi/wifi_secret.hpp"

// Теги для логирования
static const char *TAG_WIFI = "WIFI XxX";

// Настройки Wi-Fi
#ifndef WIFI_SSID
#define WIFI_SSID "Your_SSID"
#endif
#ifndef WIFI_PASS
#define WIFI_PASS "Your_PASSWORD"
#endif

#define AP_SSID "ESP32_AP"
#define AP_PASS "12345678"

static void wifi_event_handler(void *ctx, esp_event_base_t event_base,
                               int32_t event_id, void *event_data);

// ====== WIFI FUNCTIONS ======
void wifi_init(void)
{

    // Регистрация обработчика событий Wi-Fi и IP
    // Experiment
    esp_event_handler_instance_register(WIFI_EVENT, WIFI_EVENT_STA_CONNECTED, wifi_event_handler, NULL, NULL);
    esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, wifi_event_handler, NULL, NULL);

    ESP_LOGI(TAG_WIFI, "Initializing Wi-Fi...");
    esp_netif_init();

    esp_netif_create_default_wifi_ap();
    esp_netif_create_default_wifi_sta();
    // esp_netif_create_default_wifi_nan();

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

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    // ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));
    // ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &ap_config));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &sta_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_ERROR_CHECK(esp_wifi_connect());

    ESP_LOGI(TAG_WIFI, "Wi-Fi initialized. AP SSID: %s", AP_SSID);
}

static const char *TAG_WIFI_EVENT = "wifi_event XxX";

// Обработчик события для Wi-Fi
// Experiment
static void wifi_event_handler(void *ctx, esp_event_base_t event_base,
                               int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT)
    {
        switch (event_id)
        {
        case WIFI_EVENT_STA_CONNECTED:
            wifi_event_sta_connected_t *event = (wifi_event_sta_connected_t *)event_data;
            ESP_LOGI(TAG_WIFI_EVENT, "Connected to Wi-Fi network AP:");
            ESP_LOGI(TAG_WIFI_EVENT, "  SSID: %s", event->ssid);
            ESP_LOGI(TAG_WIFI_EVENT, "  BSSID: " MACSTR, MAC2STR(event->bssid));
            ESP_LOGI(TAG_WIFI_EVENT, "  Channel: %d", event->channel);
            ESP_LOGI(TAG_WIFI_EVENT, "  Auth mode: %d", event->authmode);
            break;

            // case WIFI_EVENT_STA_START:
            //     ESP_LOGI(TAG_WIFI_EVENT, "WiFi STA started");
            //     esp_wifi_connect();
            //     break;
            // case WIFI_EVENT_STA_DISCONNECTED:
            //     ESP_LOGI(TAG_WIFI_EVENT, "WiFi STA disconnected");
            //     esp_wifi_connect();
            //     break;
        }
        return;
    }

    if (event_base == IP_EVENT)
    {
        switch (event_id)
        {
        case IP_EVENT_STA_GOT_IP:
            // ESP_LOGI(TAG_WIFI_EVENT, "Got IP address");
            ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
            ESP_LOGI(TAG_WIFI_EVENT, "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));
            ESP_LOGI(TAG_WIFI_EVENT, "Netmask: " IPSTR, IP2STR(&event->ip_info.netmask));
            ESP_LOGI(TAG_WIFI_EVENT, "Gateway: " IPSTR, IP2STR(&event->ip_info.gw));
            break;
        }
    }
}