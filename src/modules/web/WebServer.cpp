#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <algorithm>
#include "esp_system.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_http_server.h"

#include "sys/queue.h"

#include "web.hpp"

// Теги для логирования
static const char *TAG_WEB_SERVER = "WEB_SERVER";
#define LOG_WEB_COLOR LOG_ANSI_COLOR_BOLD_BACKGROUND(LOG_COLOR_RED, LOG_ANSI_COLOR_BG_YELLOW)

namespace WebServer
{
#pragma region WebServer

    WebServer::WebServer() : WebServer(false) {}

    WebServer::WebServer(bool started)
    {
        ESP_LOGI(TAG_WEB_SERVER, LOG_WEB_COLOR "Constructor WebServer" LOG_ANSI_COLOR_RESET);

        if (started)
            Start();
    }

    WebServer::~WebServer()
    {
        Stop();
    }

    esp_err_t WebServer::Start()
    {
        ESP_LOGI(TAG_WEB_SERVER, LOG_WEB_COLOR "WebServer Start 1" LOG_ANSI_COLOR_RESET);

        if (_httpd_handle)
        {
            ESP_LOGI(TAG_WEB_SERVER, LOG_WEB_COLOR "WebServer Start 2" LOG_ANSI_COLOR_RESET);
            return ESP_OK;
        }

        ESP_LOGI(TAG_WEB_SERVER, LOG_WEB_COLOR "WebServer Start 4" LOG_ANSI_COLOR_RESET);
        httpd_config_t config = MakeConfig();

        ESP_LOGI(TAG_WEB_SERVER, LOG_WEB_COLOR "WebServer Start 5" LOG_ANSI_COLOR_RESET);
        _httpd_start_err = httpd_start(&_httpd_handle, &config);

        ESP_LOGI(TAG_WEB_SERVER, LOG_WEB_COLOR "WebServer Start 6" LOG_ANSI_COLOR_RESET);
        ESP_ERROR_CHECK_WITHOUT_ABORT(_httpd_start_err);

        ESP_LOGI(TAG_WEB_SERVER, LOG_WEB_COLOR "WebServer Start 7" LOG_ANSI_COLOR_RESET);
        if (_httpd_handle)
        {
            ESP_LOGI(TAG_WEB_SERVER, LOG_WEB_COLOR "WebServer Start 8" LOG_ANSI_COLOR_RESET);

            std::for_each(_handlers.begin(), _handlers.end(),
                          [](UriHandlerBase &handler)
                          {
                              ESP_LOGI(TAG_WEB_SERVER, LOG_WEB_COLOR "WebServer Start 9" LOG_ANSI_COLOR_RESET);
                              handler.Register();
                          });
        }

        return _httpd_start_err;
    }

    esp_err_t WebServer::Stop()
    {
        if (_httpd_handle)
        {
            _httpd_start_err = httpd_stop(&_httpd_handle);
            _httpd_handle = 0;
            ESP_ERROR_CHECK_WITHOUT_ABORT(_httpd_start_err);
            return _httpd_start_err;
        }
        return ESP_OK;
    }

    void WebServer::AddHandler(UriHandlerBase &handler)
    {
        _handlers.push_back(handler);

        if (_httpd_handle)
            handler.Register();
    }

#pragma endregion

#pragma region UriHandlerBase

    //// Глобальный ID для регистрации объектов
    // size_t next_id = 0;
    // std::unordered_map<size_t, std::function<esp_err_t(httpd_req_t *)>> function_registry;
    //
    //// Обёртка для C-функции (ищет объект по ID и вызывает `foo()`)
    // void wrapper_function(size_t id)
    //{
    //     if (function_registry.find(id) != function_registry.end())
    //     {
    //         function_registry[id]();
    //     }
    // }

    // **Обёртка для вызова виртуальной функции**
    esp_err_t uri_wrapper(httpd_req_t *req)
    {
        ESP_LOGI(TAG_WEB_SERVER, LOG_WEB_COLOR "Wraper %s" LOG_ANSI_COLOR_RESET, req->uri);

        UriHandlerBase *handler = static_cast<UriHandlerBase *>(req->user_ctx);
        if (handler)
        {
            return handler->Handler(req);
        }
        return ESP_FAIL;
    }

    UriHandlerBase::UriHandlerBase(WebServer &server) : UriHandlerBase(server, "/") {}

    UriHandlerBase::UriHandlerBase(WebServer &server, const char *uri) : UriHandlerBase(server, uri, "oK") {}

    UriHandlerBase::UriHandlerBase(WebServer &server, const char *uri, const char *text) : UriHandlerBase(server, uri, text, strlen(text)) {}

    UriHandlerBase::UriHandlerBase(WebServer &server, const char *uri, const char *text, ssize_t buf_len) : _webServer(server), _uri(uri), _method(HTTP_GET)
    {
        _text = text;
        _text_len = buf_len;
        _webServer.AddHandler(*this);
    }

    UriHandlerBase::~UriHandlerBase()
    {
        ESP_LOGI(TAG_WEB_SERVER, LOG_WEB_COLOR "Destructor UriHandlerBase" LOG_ANSI_COLOR_RESET);

        httpd_unregister_uri_handler(_webServer.getHandle(), _httpd_uri.uri, _httpd_uri.method);
    }

    void UriHandlerBase::Register()
    {
        _httpd_uri.uri = getUri(),
        _httpd_uri.method = getMethod(),
        _httpd_uri.handler = uri_wrapper,
        _httpd_uri.user_ctx = this;

        ESP_LOGI(TAG_WEB_SERVER, LOG_WEB_COLOR "Register UriHandlerBase %s" LOG_ANSI_COLOR_RESET, _httpd_uri.uri);

        auto err = httpd_register_uri_handler(_webServer.getHandle(), &_httpd_uri);

        ESP_LOGI(TAG_WEB_SERVER, LOG_WEB_COLOR "Register UriHandlerBase %i" LOG_ANSI_COLOR_RESET, err);
    }

    esp_err_t UriHandlerBase::Handler(httpd_req_t *req)
    {
        ESP_LOGI("TTT", "UriHandlerBase %s", req->uri);

        httpd_resp_send(req, _text, _text_len);
        return ESP_OK;
    }

#pragma endregion

} // namespace WebServer
