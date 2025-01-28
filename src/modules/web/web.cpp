#include <stdio.h>
#include <string.h>
#include "esp_system.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_http_server.h"

// Теги для логирования
static const char *TAG_WEB = "WEB";


// ====== WEB SERVER FUNCTIONS ======
static esp_err_t hello_handler(httpd_req_t *req)
{
    const char *resp = "Hello, ESP32 Web Server!";
    httpd_resp_send(req, resp, strlen(resp));
    return ESP_OK;
}

// Could it be done through events, without direct calls?
esp_err_t start_web_server(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    httpd_handle_t server = NULL;

    if (httpd_start(&server, &config) == ESP_OK)
    {
        httpd_uri_t hello_uri = {
            .uri = "/hello",
            .method = HTTP_GET,
            .handler = hello_handler,
            .user_ctx = NULL,
        };
        httpd_register_uri_handler(server, &hello_uri);
        return ESP_OK;
    }

    return ESP_FAIL;
}
