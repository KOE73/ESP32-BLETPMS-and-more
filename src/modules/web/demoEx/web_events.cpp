

#include <cstdarg>

// #include "esphome/core/log.h"
// #include "esphome/core/helpers.h"

#include "esp_log.h"

#include "esp_tls_crypto.h"

#include "utils.h"
#include "web_server_idf.h"
#include "web_events.h"

static const char *TAG_EVENT_HANDLER = "EVENT_HANDLER";
static const char *TAG_RESPONSE = "EVENT_RESPONSE";
#define LOG_WEB2_COLOR LOG_ANSI_COLOR_BOLD_BACKGROUND(LOG_COLOR_BLUE, LOG_ANSI_COLOR_BG_CYAN)

namespace esphome
{
  namespace web_server
  {

#define CRLF_STR "\r\n"
#define CRLF_LEN (sizeof(CRLF_STR) - 1)
#pragma region HandlerEventSource

    AsyncWebHandlerEventSource ::AsyncWebHandlerEventSource(std::string url) : url_(std::move(url))
    {
      _sendMutex = xSemaphoreCreateMutex();
    }

    AsyncWebHandlerEventSource::~AsyncWebHandlerEventSource()
    {
      ESP_LOGI(TAG_EVENT_HANDLER, "AsyncWebHandlerEventSource ~!~~~~~~~~~~~~~~~~~~~~~~");

      for (auto *ses : this->_event_responses)
      {
        delete ses; // NOLINT(cppcoreguidelines-owning-memory)
      }
    }

    void AsyncWebHandlerEventSource::handleRequest(AsyncWebServerRequest *request)
    {
      // Make new AsyncEventSourceResponse
      auto *rsp = new AsyncEventSourceResponse(request, this); // NOLINT(cppcoreguidelines-owning-memory)
      // External init, if any be
      if (this->on_connect_)
      {
        this->on_connect_(rsp);
      }
      // Store in sessions
      this->_event_responses.insert(rsp);
    }

    void AsyncWebHandlerEventSource::send(const char *message, const char *event, uint32_t id, uint32_t reconnect) const
    {
      if (xSemaphoreTake(_sendMutex, portMAX_DELAY))
      {
        // ESP_LOGI(TAG_EVENT_HANDLER, "AsyncWebHandlerEventSource::send sessions.count %i", this->_event_responses.size());

        for (auto *ses : this->_event_responses)
        {
          ses->send(message, event, id, reconnect);
        }
        xSemaphoreGive(_sendMutex);
      }
    }

#pragma endregion

#pragma region AsyncEventSourceResponse

    AsyncEventSourceResponse::AsyncEventSourceResponse(const AsyncWebServerRequest *request, AsyncWebHandlerEventSource *server)
        : _eventSource(server)
    {
      auto httpd_req = request->getHttpdReq();

      ESP_LOGI(TAG_RESPONSE, "AsyncEventSourceResponse constructor start");

      httpd_resp_set_status(httpd_req, HTTPD_200);
      httpd_resp_set_type(httpd_req, "text/event-stream");
      httpd_resp_set_hdr(httpd_req, "Cache-Control", "no-cache");
      httpd_resp_set_hdr(httpd_req, "Connection", "keep-alive");

      for (const auto &pair : DefaultHeaders::Instance().headers_)
      {
        httpd_resp_set_hdr(httpd_req, pair.first.c_str(), pair.second.c_str());
      }

      httpd_resp_send_chunk(httpd_req, CRLF_STR, CRLF_LEN);

      httpd_req->sess_ctx = this;
      httpd_req->free_ctx = AsyncEventSourceResponse::destroy;

      this->_httpd_handle = httpd_req->handle;
      this->_sockfd = httpd_req_to_sockfd(httpd_req);

      ESP_LOGI(TAG_RESPONSE, "AsyncEventSourceResponse constructor end sockfd = %i", _sockfd);
    }

    void AsyncEventSourceResponse::destroy(void *ptr)
    {
      ESP_LOGI(TAG_RESPONSE, "AsyncEventSourceResponse::destroy !!!!!");

      auto *rsp = static_cast<AsyncEventSourceResponse *>(ptr);
      rsp->_eventSource->_event_responses.erase(rsp);
      delete rsp; // NOLINT(cppcoreguidelines-owning-memory)
    }

    void AsyncEventSourceResponse::send(const char *message, const char *event, uint32_t id, uint32_t reconnect)
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

      // Manual analog httpd_resp_send_chunk(_httpd_handle, ev.c_str(), ev.size());

      // Sending chunked content prelude
      auto cs = str_snprintf("%x" CRLF_STR, 4 * sizeof(ev.size()) + CRLF_LEN, ev.size());
      httpd_socket_send(this->_httpd_handle, this->_sockfd, cs.c_str(), cs.size(), 0);

      // Sendiing content chunk
      httpd_socket_send(this->_httpd_handle, this->_sockfd, ev.c_str(), ev.size(), 0);

      // Indicate end of chunk
      httpd_socket_send(this->_httpd_handle, this->_sockfd, CRLF_STR, CRLF_LEN, 0);
    }

#pragma endregion

  } // namespace web_server_idf
} // namespace esphome
