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
extern const char css_css_start[] asm("_binary_css_css_start");
extern const int css_css_length;

#include "web_server_idf.h"
#include "web_server_container.h"
#include "web_handler_1.h"

using namespace web_server;

// IDFWebServer aServer(80);
WebServerContainer aServer(80);
HandlerStaticUriText index_h(aServer, "/index", index_html_start);
HandlerStaticUriBin index2_h(aServer, "/index2", index_html_gz_start, index_html_gz_length, true);
HandlerStaticUriText css_h(aServer, "/css.css", css_css_start, css_css_length);
AsyncWebHandlerEventSource events_h(aServer, "/events");
AsyncWebHandlerWSSource ws_h(aServer, "/ws");

// Could it be done through events, without direct calls?
esp_err_t start_web_server(void)
{

    aServer.set_auth_username("1");
    aServer.set_auth_password("1");

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

    xTaskCreate(
        [](void *param)
        {
            ESP_LOGI(TAG_WEB, "Task EVENT started!");
            int c = 0;
            while (true)
            {
                std::string ss = "step: ";
                ss.append(std::to_string(c));

                ESP_LOGI(TAG_WEB, "Task EVENT is running... [%s]", ss.c_str());

                events_h.send(ss.c_str());
                ws_h.send(ss.c_str());
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
