#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include "esp_system.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_http_server.h"

#include "sys/queue.h"

#include "web.hpp"

#include "ArduinoJson.h"
#include "yabt.hpp"
#include "yabt_tpms.hpp"
#include "modules/diagnostic/diagnostic.hpp"

// #include "WebServer.hpp"

// using WebServer::WebServer;

/*
    https://docs.platformio.org/en/latest/platforms/espressif32.html#embedding-binary-data

    1. Using embed (from C++17)
        #include "incbin.h"
        INCBIN(style_css, "path/to/style.css");
    2. Using xxd to convert a file to a byte array
    3. Using the esp_embed component (for ESP-IDF)
        makefile -> COMPONENT_EMBED_FILES := style.css

        extern const uint8_t style_css_start[] asm("_binary_style_css_start");
        extern const uint8_t style_css_end[] asm("_binary_style_css_end");

        void app_main() {
            size_t style_css_size = style_css_end - style_css_start;
            printf("CSS file size: %zu\n", style_css_size);
            printf("CSS content: %.*s\n", style_css_size, style_css_start);
        }
    4. Using SPIFFS or LittleFS
*/

// Теги для логирования
static const char *TAG_WEB = "WEB XxX";

extern const char index_html_start[] asm("_binary_index_html_start");
extern const int index_html_length;
extern const uint8_t index_html_gz_start[] asm("_binary_index_html_gz_start");
extern const int index_html_gz_length;

extern const uint8_t ble_html_gz_start[] asm("_binary_ble_html_gz_start");
extern const int ble_html_gz_length;

extern const char css_css_start[] asm("_binary_css_css_start");
extern const int css_css_length;

#include "web_server_idf.h"
#include "web_server_container.h"
#include "handler_static.h"
#include "handler_events.h"
#include "handler_ws.h"
#include "handler_api.h"
#include "inner_events.h"

__attribute__((weak)) esp_event_loop_handle_t CBOR_loop = NULL;
__attribute__((weak)) esp_event_base_t CBOR_EVENT = NULL;

using namespace yaidfws;

// IDFWebServer aServer(80);
WebServerContainer aServer(80);
// HandlerStaticUriText index_main(aServer, "/", index_html_start);
HandlerStaticUriText index_h(aServer, "/index", index_html_start);
HandlerStaticUriBin index2_h(aServer, "/index2", index_html_gz_start, index_html_gz_length, true);
HandlerStaticUriBin ble_h(aServer, "/ble", ble_html_gz_start, ble_html_gz_length, true);
HandlerStaticUriText css_h(aServer, "/css.css", css_css_start, css_css_length);
AsyncWebHandlerEventSource events_h(aServer, "/events");
AsyncWebHandlerWSSource ws_h(aServer, "/ws");

HandlerApi handler_api; // aServer

void event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    if (event_base == SYS_INNER_EVENT)
    {
        switch (event_id)
        {
        case SYS_INNER_EVENT_WS_SEND_JSON:
            // ESP_LOGI(TAG_WEB, "SYS_INNER_EVENT_BASE:SYS_INNER_EVENT_WS_SEND_JSON");
            const char *json_str = static_cast<const char *>(event_data);
            ws_h.send(json_str);
            break;
        }
    }
    // ESP_HTTP_SERVER_EVENT
    // WIFI_EVENT
}

void event_handler_any(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    if (event_base == SYS_INNER_EVENT)
        return;
    ESP_LOGI("EV_XxX", "%s : %li", event_base, event_id);
}

/// @brief  Converts BLE events to JSON and sends them to the web.
void yabt_handler_any(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    if (event_id == YABT_EVENT_TPMS)
    {
        ESP_LOGI("EV_XxX", "Receive YABT_EVENT_TPMS");
        if (!ws_h.count())
            return;

        yabt::TPMSData &data = *((yabt::TPMSData *)event_data);
        JsonDocument json;

        json["msgType"] = "tpms";
        json["msgSource"] = "ble";
        json["id"] = data.id;
        // ASCII UTF ???? manufacturerName json["manufacturerName"] = data.manufacturerName;
        json["sensorNumber"] = data.sensorNumber;
        json["sensorAddress"] = data.sensorAddress;
        json["pressureRaw"] = data.pressureRaw;
        json["temperatureRaw"] = data.temperatureRaw;
        json["batteryPercentage"] = data.batteryPercentage;
        json["alarmFlag"] = data.alarmFlag;
        json["pressure_kPa"] = data.pressure_kPa;
        json["pressure_mbar"] = data.pressure_mbar;
        json["pressure_Psi"] = data.pressure_Psi;
        json["pressure_Bar"] = data.pressure_Bar;
        json["pressure_KgCm2"] = data.pressure_KgCm2;
        json["pressure_Atm"] = data.pressure_Atm;
        json["temperatureC"] = data.temperatureC;
        json["temperatureF"] = data.temperatureF;

        std::string json_str;
        serializeJson(json, json_str);

        ws_h.send(json_str.c_str());


        

        return;
    }
}

/// @brief  Converts BLE events to JSON and sends them to the web.
void cbor_handler_any(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    ESP_LOGI("EV_XxX", "Web Receive CBOR");

    size_t len = (size_t)event_id;
    uint8_t *buf = (uint8_t *)event_data;
    ESP_LOGI("EV_XxX", "Web CBOR data size: %u bytes", len);

    ws_h.send_binary(buf, len);
    // ws_h.send(buf, len);
}

// Could it be done through events, without direct calls?
esp_err_t start_web_server(void)
{

    aServer.set_auth_username("1");
    aServer.set_auth_password("1");

    aServer.add_handler(&handler_api);

    aServer.start();

    events_h.onConnect([](AsyncEventSourceResponse *client)
                       {
                           ESP_LOGI(TAG_WEB, "Events -> onConnect");
                           client->send("Hello on onConnect"); });
    ws_h.onConnect([](AsyncWSSourceResponse *client)
                   {
                       ESP_LOGI(TAG_WEB, "WS -> onConnect");
                       // client->send("Hello on onConnect");
                   });

    esp_event_handler_register(SYS_INNER_EVENT, ESP_EVENT_ANY_ID, event_handler, NULL);
    esp_event_handler_register(ESP_EVENT_ANY_BASE, ESP_EVENT_ANY_ID, event_handler_any, NULL);
    esp_event_handler_register_with(yabt::BTController::getInstance().getEventLoop(), YABT_EVENT, ESP_EVENT_ANY_ID, yabt_handler_any, NULL);

    ESP_LOGI(TAG_WEB, "esp_event_handler_register_with before test . ");

    if (CBOR_loop != NULL)
    {
        ESP_LOGI(TAG_WEB, "esp_event_handler_register_with . %s", CBOR_EVENT);
        esp_event_handler_register_with(CBOR_loop, CBOR_EVENT, ESP_EVENT_ANY_ID, cbor_handler_any, NULL);
    }

    xTaskCreate(
        [](void *param)
        {
            ESP_LOGI(TAG_WEB, "Task EVENT started!");
            int c = 0;

            JsonDocument json;

            std::string output;

            while (true)
            {
                std::string ss = "{\"step\":";
                ss.append(std::to_string(c));
                ss.append("}");

                // ESP_LOGI(TAG_WEB, "Task EVENT is running... [%s]", ss.c_str());

                events_h.send(ss.c_str());
                ws_h.send(ss.c_str());

                json["msgType"] = "webTick";
                json["id"] = c;

                serializeJson(json, output);

                events_h.send(output.c_str());
                ws_h.send(output.c_str());
                ws_h.send(print_task_diagnostics_json().c_str());

                vTaskDelay(pdMS_TO_TICKS(3000));
                c++;
            }
        },
        "LambdaTask", // Имя задачи
        4096,         // Размер стека
        nullptr,      // Параметры
        5,            // Приоритет
        nullptr       // Handle (можно оставить nullptr, если не нужно)
    );

    return ESP_OK;
}
