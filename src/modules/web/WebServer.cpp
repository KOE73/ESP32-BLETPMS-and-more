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

// Теги для логирования
static const char *TAG_WEB = "WEB_SERVER";

namespace WebServer
{
#pragma region WebServer

    WebServer::WebServer(bool started)
    {
        if (started)
            Start();
    }

    WebServer::WebServer() : WebServer(false) {}

    WebServer::~WebServer()
    {
        if (_httpd_start_err == ESP_OK)
            httpd_stop(_httpd_handle);
    }

    esp_err_t WebServer::Start()
    {
        httpd_config_t config = MakeConfig();
        _httpd_start_err = httpd_start(&_httpd_handle, &config);
        ESP_ERROR_CHECK_WITHOUT_ABORT(_httpd_start_err);
        return _httpd_start_err;
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
        ESP_LOGI(TAG_WEB, "Wraper %s", req->uri);

        UriHandlerBase *handler = static_cast<UriHandlerBase *>(req->user_ctx);
        if (handler)
        {
            return handler->Handler(req);
        }
        return ESP_FAIL;
    }
    UriHandlerBase::UriHandlerBase(WebServer &server) : UriHandlerBase(server, "/") {}

    UriHandlerBase::UriHandlerBase(WebServer &server, const char *uri) : _webServer(server), _uri(uri), _method(HTTP_GET)
    {
        ESP_LOGI(TAG_WEB, "Constructor UriHandlerBase %s", _uri);

        _httpd_uri.uri = _uri,
        _httpd_uri.method = _method,
        _httpd_uri.handler = uri_wrapper,
        _httpd_uri.user_ctx = this;
        httpd_register_uri_handler(_webServer.getHandle(), &_httpd_uri);
    }

    UriHandlerBase::~UriHandlerBase()
    {
        ESP_LOGI(TAG_WEB, "Destructor UriHandlerBase");

        httpd_unregister_uri_handler(_webServer.getHandle(), _httpd_uri.uri, _httpd_uri.method);
    }

#pragma endregion

} // namespace WebServer
