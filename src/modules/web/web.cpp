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
#include "WebServer.hpp"

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

WebServer::WebServer webServer;
WebServer::UriHandlerBase x(webServer);
WebServer::UriHandlerBase index_(webServer, "/index", index_html_start);
WebServer::UriHandlerBase css_(webServer, "/css.css", css_css_start, css_css_length);

#include "demoEx/web_server_idf.h"
#include "demoEx/web_server_container.h"
#include "demoEx/web_handler_1.h"

using namespace web_server;

// IDFWebServer aServer(80);
WebServerContainer aServer(80);
AsyncWebHandler_1 index_h("/index", index_html_start);
AsyncWebHandler_2 index2_h("/index2", index_html_gz_start, index_html_gz_length,true);
AsyncWebHandler_1 css_h("/css.css", css_css_start, css_css_length);

//  AsyncWebServer server(80);
//  // Создание WebSocket обработчика
//  AsyncWebSocket ws("/ws");
//  // Отправка SSE-сообщений каждую секунду
//  unsigned long lastMillis = 0;
//  AsyncWebHandlerEventSource events("/events");

#pragma region Events

#define MAX_CLIENTS 5

typedef struct client_session
{
    httpd_req_t *req;
    httpd_handle_t handle;
    int fd;
    STAILQ_ENTRY(client_session)
    entry;
} client_session_t;

STAILQ_HEAD(client_sessions, client_session)
clients;
static struct client_sessions active_clients;
static pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

void send_log_to_clients(const char *log)
{
    char sse_data[512];
    snprintf(sse_data, sizeof(sse_data), "data: %s\n\n", log);

    pthread_mutex_lock(&clients_mutex);

    client_session_t *client;
    STAILQ_FOREACH(client, &active_clients, entry)
    {
        // httpd_send(client->req, sse_data, strlen(sse_data));
        httpd_resp_send_chunk(client->req, sse_data, strlen(sse_data));
    }

    pthread_mutex_unlock(&clients_mutex);
}

static vprintf_like_t original_vprintf = NULL; // Сохраняем стандартный vprintf

static int custom_vprintf(const char *fmt, va_list args)
{
    char log_msg[256];
    int len = vsnprintf(log_msg, sizeof(log_msg), fmt, args);

    printf("\033[03;38;05;222msend_log_to_clients %s.\033[0m", log_msg);

    send_log_to_clients(log_msg);

    // Дублирование в стандартный vprintf (консоль)
    if (original_vprintf)
    {
        printf("\033[03;38;05;222moriginal_vprintf %s.\033[0m", log_msg);

        return original_vprintf(fmt, args);
    }

    return len;
}

static esp_err_t logs_get_handler(httpd_req_t *req)
{
    // ESP_LOGI(TAG_WEB,"Logs 1");
    printf("\033[03;38;05;222mLogs 1.\033[0m\n");

    // Устанавливаем заголовки SSE
    httpd_resp_set_type(req, "text/event-stream");
    httpd_resp_set_hdr(req, "Cache-Control", "no-cache");
    httpd_resp_set_hdr(req, "Connection", "keep-alive");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*"); // ?

    // Добавляем нового клиента в список
    client_session_t *client = (client_session_t *)malloc(sizeof(client_session_t));
    client->req = req;
    client->handle = req->handle;
    client->fd = httpd_req_to_sockfd(req);

    // ESP_LOGI(TAG_WEB, "Logs 2");
    printf("\033[03;38;05;222mLogs 2.\033[0m\n");

    pthread_mutex_lock(&clients_mutex);
    STAILQ_INSERT_TAIL(&active_clients, client, entry);
    pthread_mutex_unlock(&clients_mutex);

    // ESP_LOGI(TAG_WEB, "Logs 3");
    printf("\033[03;38;05;222mLogs 3.\033[0m\n");

    // Удерживаем соединение открытым
    while (true)
    {
        // ESP_LOGI(TAG_WEB, "Logs 4");
        printf("\033[03;38;05;222mLogs 4.\033[0m\n");

        // Отправляем данные клиенту
        if (httpd_resp_send_chunk(req, "tick\n", HTTPD_RESP_USE_STRLEN) != ESP_OK)
        {
            ESP_LOGE(TAG_WEB, "Failed to send SSE data");
            break;
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    // ESP_LOGI(TAG_WEB, "Logs 5");
    printf("\033[03;38;05;222mLogs 5.\033[0m\n");

    // Удаляем клиента из списка
    pthread_mutex_lock(&clients_mutex);
    STAILQ_REMOVE(&active_clients, client, client_session, entry);
    pthread_mutex_unlock(&clients_mutex);

    free(client);

    return ESP_OK;
}

#pragma endregion Events

// ====== WEB SERVER FUNCTIONS ======
static esp_err_t hello_get_handler(httpd_req_t *req)
{
    const char *resp = "Hello, ESP32 Web Server!";
    httpd_resp_send(req, resp, strlen(resp));
    return ESP_OK;
}

static esp_err_t index_get_handler(httpd_req_t *req)
{
    httpd_resp_send(req, index_html_start, index_html_length);
    return ESP_OK;
}

static esp_err_t css_get_handler(httpd_req_t *req)
{
    httpd_resp_send(req, css_css_start, css_css_length);
    return ESP_OK;
}

static const httpd_uri_t uri_list[] = {
    {.uri = "/index", .method = HTTP_GET, .handler = index_get_handler},
    {.uri = "/", .method = HTTP_GET, .handler = index_get_handler},
    {.uri = "/hello", .method = HTTP_GET, .handler = hello_get_handler},
    {.uri = "/css.css", .method = HTTP_GET, .handler = css_get_handler},
    {.uri = "/logs", .method = HTTP_GET, .handler = logs_get_handler},
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
    // httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    // httpd_handle_t server = NULL;
    //
    // if (httpd_start(&server, &config) != ESP_OK)
    //{
    //    return ESP_FAIL;
    //}
    // webServer.AddUriHandler(new );

    // webServer = new WebServer::WebServer (true);
    // x = new WebServer::UriHandlerBase(*webServer);

    // ESP_LOGI(TAG_WEB, "WebServer Starting");
    //
    // if (webServer.Start() != ESP_OK)
    //{
    //    ESP_LOGI(TAG_WEB, "WebServer not started");
    //    return ESP_FAIL;
    //}
    // ESP_LOGI(TAG_WEB, "WebServer Started");

    // aServer.begin();
    aServer.setup();
    aServer.add_handler(&index_h);
    aServer.add_handler(&index2_h);
    aServer.add_handler(&css_h);

    //for (int i = 0; i < 50; i++)
    //{
    //    printf("to EVENT");
    //    aServer.getEnents();
    //    vTaskDelay(pdMS_TO_TICKS(2123));
    //}

    xTaskCreate(
        [](void *param)
        {
            ESP_LOGI(TAG_WEB, "Task EVENT started!");
            int c=0;
            while (true)
            {
                ESP_LOGI(TAG_WEB, "Task EVENT is running...");
                aServer.getEvents().send("!!!");
                aServer.getWS().send("ww");
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

    // register_routes(server);

    // Инициализация списка клиентов
    STAILQ_INIT(&active_clients);

    // Регистрация кастомного обработчика логов
    // original_vprintf = esp_log_set_vprintf(custom_vprintf);

    return ESP_OK;
}
