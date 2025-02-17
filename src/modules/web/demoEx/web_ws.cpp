

#include <cstdarg>

// #include "esphome/core/log.h"
// #include "esphome/core/helpers.h"

#include "esp_log.h"

#include "esp_tls_crypto.h"

#include "utils.h"
#include "web_server_idf.h"
#include "web_ws.h"

static const char *TAG_EVENT_HANDLER = "EVENT_HANDLER";
static const char *TAG_RESPONSE = "WS_RESPONSE";
#define LOG_WEB2_COLOR LOG_ANSI_COLOR_BOLD_BACKGROUND(LOG_COLOR_BLUE, LOG_ANSI_COLOR_BG_CYAN)

namespace web_server
{

#define CRLF_STR "\r\n"
#define CRLF_LEN (sizeof(CRLF_STR) - 1)
#pragma region HandlerEventSource

    AsyncWebHandlerWSSource::AsyncWebHandlerWSSource(std::string url) : url_(std::move(url))
    {
        _sendMutex = xSemaphoreCreateMutex();
    }

    AsyncWebHandlerWSSource::~AsyncWebHandlerWSSource()
    {
        ESP_LOGI(TAG_EVENT_HANDLER, "AsyncWebHandlerWSSource ~!~~~~~~~~~~~~~~~~~~~~~~");

        for (auto *ses : this->_ws_responses)
        {
            delete ses; // NOLINT(cppcoreguidelines-owning-memory)
        }
    }

    bool AsyncWebHandlerWSSource::canHandle(AsyncWebServerRequest *request)
    {
        ESP_LOGI(TAG_RESPONSE, "AsyncWebHandlerWSSource::canHandle 1 method=%i url='%s'", request->method(), request->url().c_str());

        auto httpd_req = request->getHttpdReq();

        if (httpd_req->method == HTTP_GET && request->url() == url_)
        {
            ESP_LOGI(TAG_RESPONSE, "AsyncWebHandlerWSSource::canHandle 2 method=%i url='%s'", request->method(), request->url().c_str());
            return true;
        }

        if (httpd_req->method == 0)
        {
            ESP_LOGI(TAG_RESPONSE, "AsyncWebHandlerWSSource::canHandle 3 method=%i url='%s'", request->method(), request->url().c_str());
            return true;
        }

        return false;
    }

    void AsyncWebHandlerWSSource::handleRequest(AsyncWebServerRequest *request)
    {
        auto httpd_req = request->getHttpdReq();

        if (httpd_req->method == HTTP_GET)
        {                                                         // Make new AsyncWSSourceResponse
            auto *rsp = new AsyncWSSourceResponse(request, this); // NOLINT(cppcoreguidelines-owning-memory)
            httpd_req->user_ctx = (void *)_key;

            this->_ws_responses.insert(rsp);

            ESP_LOGI(TAG_RESPONSE, "AsyncWebHandlerWSSource::handleRequest HTTP_GET");
            
             //httpd_ws_get_fd_info(httpd_req);
            return;
        }
        //// External init, if any be
        // if (this->on_connect_)
        //{
        //   this->on_connect_(rsp);
        // }
        //  Store in sessions

        const char *connection_type = (const char *)httpd_req->user_ctx;

        ESP_LOGI(TAG_RESPONSE, "AsyncWebHandlerWSSource::handleRequest '%s' '%s'", connection_type, _key);

        // Буфер для входящих данных
        char buf[128];
        httpd_ws_frame_t ws_pkt;
        memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
        ws_pkt.type = HTTPD_WS_TYPE_TEXT;
        ws_pkt.payload = (uint8_t *)buf;

        // Читаем WebSocket-сообщение
        esp_err_t ret = httpd_ws_recv_frame(httpd_req, &ws_pkt, sizeof(buf));
        if (ret != ESP_OK)
        {
            ESP_LOGE(TAG_RESPONSE, "Error receiving WebSocket frame: %d", ret);
            return;
        }

        buf[ws_pkt.len] = '\0'; // Завершаем строку
        ESP_LOGI(TAG_RESPONSE, "Received: %s", buf);
    }

    void AsyncWebHandlerWSSource::send(const char *message, const char *event, uint32_t id, uint32_t reconnect) const
    {
        if (xSemaphoreTake(_sendMutex, portMAX_DELAY))
        {
            // ESP_LOGI(TAG_EVENT_HANDLER, "AsyncWebHandlerWSSource::send sessions.count %i", this->_event_responses.size());

            for (auto *ses : this->_ws_responses)
            {
                ses->send(message, event, id, reconnect);
            }
            xSemaphoreGive(_sendMutex);
        }
    }

#pragma endregion

#pragma region AsyncWSSourceResponse

    AsyncWSSourceResponse::AsyncWSSourceResponse(const AsyncWebServerRequest *request, AsyncWebHandlerWSSource *server)
        : _eventSource(server)
    {
        auto httpd_req = request->getHttpdReq();

        if (httpd_req->method == HTTP_GET)
        {
            // Это WebSocket Handshake
            ESP_LOGI(TAG_RESPONSE, "WebSocket handshake initiated");

            httpd_req->sess_ctx = this;
            httpd_req->free_ctx = AsyncWSSourceResponse::destroy;

            this->_httpd_handle = httpd_req->handle;
            this->_sockfd = httpd_req_to_sockfd(httpd_req);

            ESP_LOGI(TAG_RESPONSE, "AsyncWSSourceResponse HTTP_GET sockfd = %i", _sockfd);

            // return ESP_OK;
        }
    }

    void AsyncWSSourceResponse::destroy(void *ptr)
    {
        ESP_LOGI(TAG_RESPONSE, "AsyncWSSourceResponse::destroy !!!!!");

        auto *rsp = static_cast<AsyncWSSourceResponse *>(ptr);
        rsp->_eventSource->_ws_responses.erase(rsp);
        delete rsp; // NOLINT(cppcoreguidelines-owning-memory)
    }

    void AsyncWSSourceResponse::send(const char *message, const char *event, uint32_t id, uint32_t reconnect)
    {
        if (this->_sockfd == 0)
        {
            return;
        }

        std::string ev;

        ////if (reconnect)
        ////{
        ////  ev.append("retry: ", sizeof("retry: ") - 1);
        ////  ev.append(to_string(reconnect));
        ////  ev.append(CRLF_STR, CRLF_LEN);
        ////}
        ////
        ////if (id)
        ////{
        ////  ev.append("id: ", sizeof("id: ") - 1);
        ////  ev.append(to_string(id));
        ////  ev.append(CRLF_STR, CRLF_LEN);
        ////}

        // if (event && *event)
        //{
        //   ev.append("event: ", sizeof("event: ") - 1);
        //   ev.append(event);
        //   ev.append(CRLF_STR, CRLF_LEN);
        // }

        if (message && *message)
        {
            ev.append("data: ", sizeof("data: ") - 1);
            ev.append(message);
            ev.append(CRLF_STR, CRLF_LEN);
        }

        if (ev.empty())
        {
            return;
        }

        ev.append(CRLF_STR, CRLF_LEN);

        // Отправляем ответ обратно клиенту
        httpd_ws_frame_t ws_res;
        memset(&ws_res, 0, sizeof(httpd_ws_frame_t));
        ws_res.type = HTTPD_WS_TYPE_TEXT;
        ws_res.payload = (uint8_t *)ev.c_str();
        ws_res.len = ev.size();

        auto err = httpd_ws_send_data(this->_httpd_handle, this->_sockfd, &ws_res);
        // httpd_ws_send_data_async(req, &ws_res);
        // httpd_ws_send_frame(req, &ws_res);

        ESP_LOGI(TAG_RESPONSE, "httpd_ws_send_data %i", err);
    }

#pragma endregion

} // namespace web_server_idf
