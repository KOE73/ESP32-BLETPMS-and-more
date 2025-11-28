#include <stdio.h>
#include <string.h>
#include <memory_resource>
#include "esp_system.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_mac.h"
// #include "esp_http_server.h"

#include <nlohmann/json.hpp>

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

#define AP_SSID "TPMS_AP"
#define AP_PASS "12345678"

__attribute__((weak)) esp_event_loop_handle_t CBOR_loop = NULL;
__attribute__((weak)) esp_event_base_t CBOR_EVENT = NULL;

static void wifi_event_handler(void *ctx, esp_event_base_t event_base,
                               int32_t event_id, void *event_data);

// ====== WIFI FUNCTIONS ======
void wifi_init(void)
{

    // Регистрация обработчика событий Wi-Fi и IP
    // Experiment
    esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_event_handler, NULL, NULL);
    esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_event_handler, NULL, NULL);
    esp_event_handler_instance_register(IP_EVENT, ESP_EVENT_ANY_ID, wifi_event_handler, NULL, NULL);

    ESP_LOGI(TAG_WIFI, "Initializing Wi-Fi...");
    esp_netif_init();

    // esp_netif_create_default_wifi_ap();
    esp_netif_create_default_wifi_sta();
    // esp_netif_create_default_wifi_nan();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);

    // Отключение энергосбережения для стабильного соединения
    // ChatGPT Это самое важное для стабильной работы Wi-Fi + BLE одновременно.
    esp_wifi_set_ps(WIFI_PS_NONE);

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
            .scan_method = WIFI_FAST_SCAN, // WIFI_ALL_CHANNEL_SCAN
            .failure_retry_cnt = 5,
        }};

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    // ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));
    // ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &ap_config));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &sta_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    // ESP_ERROR_CHECK(esp_wifi_connect());

    ESP_LOGI(TAG_WIFI, "Wi-Fi initialized. AP SSID: %s. STA SSID: %s", AP_SSID, sta_config.sta.ssid);
}

static const char *TAG_WIFI_EVENT = "wifi_event XxX";
static int s_retry_num = 0;
static const int MAX_RETRY = 10;

// Обработчик события для Wi-Fi
// Experiment
static void wifi_event_handler(void *ctx, esp_event_base_t event_base,
                               int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT)
    {
        switch (event_id)
        {
        case WIFI_EVENT_STA_START:
        {
            ESP_LOGI(TAG_WIFI_EVENT, "WiFi STA started");
            ESP_ERROR_CHECK_WITHOUT_ABORT(esp_wifi_connect());
            break;
        }

        case WIFI_EVENT_STA_CONNECTED:
        {
            wifi_event_sta_connected_t *event = (wifi_event_sta_connected_t *)event_data;
            ESP_LOGI(TAG_WIFI_EVENT, "Connected to Wi-Fi network AP:");
            ESP_LOGI(TAG_WIFI_EVENT, "  SSID: %s", event->ssid);
            ESP_LOGI(TAG_WIFI_EVENT, "  BSSID: " MACSTR, MAC2STR(event->bssid));
            ESP_LOGI(TAG_WIFI_EVENT, "  Channel: %d", event->channel);
            ESP_LOGI(TAG_WIFI_EVENT, "  Auth mode: %d", event->authmode);
            break;
        }

        case WIFI_EVENT_STA_DISCONNECTED:
        {
            if (s_retry_num < MAX_RETRY)
            {
                ESP_LOGW("wifi", "Disconnected, retrying... (%d/%d)", s_retry_num + 1, MAX_RETRY);
                vTaskDelay(pdMS_TO_TICKS(500)); // Дать BLE/радио чуть отдышаться
                ESP_ERROR_CHECK_WITHOUT_ABORT(esp_wifi_connect());
                s_retry_num++;
            }
            else
            {
                ESP_LOGE("wifi", "Failed to connect after %d retries", MAX_RETRY);
            }
            break;
        }
        default:
            ESP_LOGI(TAG_WIFI_EVENT, "Unhandled WiFi event: %d", event_id);
            break;
        }
        return;
    }

    if (event_base == IP_EVENT)
    {
        switch (event_id)
        {
        case IP_EVENT_STA_GOT_IP:
        {
            // ESP_LOGI(TAG_WIFI_EVENT, "Got IP address");
            ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
            ESP_LOGI(TAG_WIFI_EVENT, "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));
            ESP_LOGI(TAG_WIFI_EVENT, "Netmask: " IPSTR, IP2STR(&event->ip_info.netmask));
            ESP_LOGI(TAG_WIFI_EVENT, "Gateway: " IPSTR, IP2STR(&event->ip_info.gw));

            if (CBOR_loop != NULL)
            {
                nlohmann::json j;
                j["cbor"] = "IP";
                char buf[IP4ADDR_STRLEN_MAX];
                esp_ip4addr_ntoa(&event->ip_info.ip, buf, IP4ADDR_STRLEN_MAX);
                j["IP"] = buf;

                ESP_LOGI(TAG_WIFI_EVENT, "CBOR_loop send: %s", buf);

                std::array<std::byte, 100> stack_buffer;
                std::pmr::monotonic_buffer_resource mempool{stack_buffer.data(), stack_buffer.size(), std::pmr::new_delete_resource()};
                auto out = std::pmr::vector<std::uint8_t>(&mempool);
                out.reserve(100);
                nlohmann::json::to_cbor(j, out);
                ESP_ERROR_CHECK_WITHOUT_ABORT(esp_event_post_to(
                    CBOR_loop,
                    CBOR_EVENT,
                    out.size(),
                    out.data(),
                    out.size(),
                    pdMS_TO_TICKS(3000)));
            }
            break;
        }
        default:
        {
            ESP_LOGI(TAG_WIFI_EVENT, "Unhandled IP event: %d", event_id);
            break;
        }
        }
    }
}