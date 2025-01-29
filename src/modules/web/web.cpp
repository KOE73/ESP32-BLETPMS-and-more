#include <stdio.h>
#include <string.h>
#include "esp_system.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_http_server.h"

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
static const char *TAG_WEB = "WEB";

// ====== WEB SERVER FUNCTIONS ======
static esp_err_t hello_get_handler(httpd_req_t *req)
{
    const char *resp = "Hello, ESP32 Web Server!";
    httpd_resp_send(req, resp, strlen(resp));
    return ESP_OK;
}

static esp_err_t main_get_handler(httpd_req_t *req)
{
    const char *resp = "<html><head><link rel='stylesheet' href='css.css' type='text/css'></head><body>It's body! Hello, ESP32 Web Server!</body></div></html>";
    httpd_resp_send(req, resp, strlen(resp));
    return ESP_OK;
}

extern const int index_html_length;
extern const int css_css_length;

//extern const uint8_t index_html_start[] asm("_binary_index_html_start");
//extern const uint8_t index_html_end[] asm("_binary_index_html_end");
extern const char css_css_start[] asm("_binary_css_css_start");
extern const char index_html_start[] asm("_binary_index_html_start");
//extern const char index_html_end[] asm("_binary_index_html_end");
//extern const char *index_html asm("index_html");
//extern const char *css_css asm("css_css");


static esp_err_t index_get_handler(httpd_req_t *req)
{
    //size_t index_html_size = index_html_end - index_html_start;
    printf("CSS file size: %zu\n", index_html_length);
    printf("CSS content: %.*s\n", index_html_length, index_html_start);

    httpd_resp_send(req, index_html_start, index_html_length);
    return ESP_OK;
}

static esp_err_t css_get_handler(httpd_req_t *req)
{
    //const char *resp = "body {font-size: xxx-large;}";
    //httpd_resp_send(req, resp, strlen(resp));
    httpd_resp_send(req, css_css_start, css_css_length);
    return ESP_OK;
}

static const httpd_uri_t uri_list[] = {
    {.uri = "/index", .method = HTTP_GET, .handler = index_get_handler},
    {.uri = "/", .method = HTTP_GET, .handler = index_get_handler},
    {.uri = "/hello", .method = HTTP_GET, .handler = hello_get_handler},
    {.uri = "/css.css", .method = HTTP_GET, .handler = css_get_handler},
    //{ .uri = "/status", .method = HTTP_GET, .handler = status_handler },
    //{ .uri = "/post", .method = HTTP_POST, .handler = post_handler }
};

static void register_routes(httpd_handle_t server)
{
    for (size_t i = 0; i < sizeof(uri_list) / sizeof(uri_list[0]); i++)
    {
        httpd_register_uri_handler(server, &uri_list[i]);
    }
}

// Could it be done through events, without direct calls?
esp_err_t start_web_server(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    httpd_handle_t server = NULL;

    if (httpd_start(&server, &config) == ESP_OK)
    {
        register_routes(server);
        // httpd_uri_t hello_uri = {
        //     .uri = "/hello",
        //     .method = HTTP_GET,
        //     .handler = hello_get_handler,
        //     .user_ctx = NULL,
        // };
        // httpd_register_uri_handler(server, &hello_uri);
        return ESP_OK;
    }

    return ESP_FAIL;
}
