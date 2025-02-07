

#include <cstdarg>

// #include "esphome/core/log.h"
// #include "esphome/core/helpers.h"

#include "esp_log.h"

#include "esp_tls_crypto.h"

#include "utils.h"
#include "web_server_idf.h"
#include "web_events.h"

static const char *TAG_WEB2_SERVER = "WEB2_SERVER";
#define LOG_WEB2_COLOR LOG_ANSI_COLOR_BOLD_BACKGROUND(LOG_COLOR_BLUE, LOG_ANSI_COLOR_BG_CYAN)

namespace esphome
{
  namespace web_server_idf
  {

#define CRLF_STR "\r\n"
#define CRLF_LEN (sizeof(CRLF_STR) - 1)

    AsyncWebHandlerEventSource::~AsyncWebHandlerEventSource()
    {
      for (auto *ses : this->sessions_)
      {
        delete ses; // NOLINT(cppcoreguidelines-owning-memory)
      }
    }

    void AsyncWebHandlerEventSource::handleRequest(AsyncWebServerRequest *request)
    {
      auto *rsp = new AsyncEventSourceResponse(request, this); // NOLINT(cppcoreguidelines-owning-memory)
      if (this->on_connect_)
      {
        this->on_connect_(rsp);
      }
      this->sessions_.insert(rsp);
    }

    void AsyncWebHandlerEventSource::send(const char *message, const char *event, uint32_t id, uint32_t reconnect)
    {
      for (auto *ses : this->sessions_)
      {
        ses->send(message, event, id, reconnect);
      }
    }

    AsyncEventSourceResponse::AsyncEventSourceResponse(const AsyncWebServerRequest *request, AsyncWebHandlerEventSource *server)
        : _eventSource(server)
    {
      httpd_req_t *req = *request;

      httpd_resp_set_status(req, HTTPD_200);
      httpd_resp_set_type(req, "text/event-stream");
      httpd_resp_set_hdr(req, "Cache-Control", "no-cache");
      httpd_resp_set_hdr(req, "Connection", "keep-alive");

      for (const auto &pair : DefaultHeaders::Instance().headers_)
      {
        httpd_resp_set_hdr(req, pair.first.c_str(), pair.second.c_str());
      }

      httpd_resp_send_chunk(req, CRLF_STR, CRLF_LEN);

      req->sess_ctx = this;
      req->free_ctx = AsyncEventSourceResponse::destroy;

      this->_httpd_handle = req->handle;
      this->fd_ = httpd_req_to_sockfd(req);
    }

    void AsyncEventSourceResponse::destroy(void *ptr)
    {
      auto *rsp = static_cast<AsyncEventSourceResponse *>(ptr);
      rsp->_eventSource->sessions_.erase(rsp);
      delete rsp; // NOLINT(cppcoreguidelines-owning-memory)
    }

    void AsyncEventSourceResponse::send(const char *message, const char *event, uint32_t id, uint32_t reconnect)
    {
      if (this->fd_ == 0)
      {
        return;
      }

      std::string ev;

      if (reconnect)
      {
        ev.append("retry: ", sizeof("retry: ") - 1);
        ev.append(to_string(reconnect));
        ev.append(CRLF_STR, CRLF_LEN);
      }

      if (id)
      {
        ev.append("id: ", sizeof("id: ") - 1);
        ev.append(to_string(id));
        ev.append(CRLF_STR, CRLF_LEN);
      }

      if (event && *event)
      {
        ev.append("event: ", sizeof("event: ") - 1);
        ev.append(event);
        ev.append(CRLF_STR, CRLF_LEN);
      }

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

      // Sending chunked content prelude
      auto cs = str_snprintf("%x" CRLF_STR, 4 * sizeof(ev.size()) + CRLF_LEN, ev.size());
      httpd_socket_send(this->_httpd_handle, this->fd_, cs.c_str(), cs.size(), 0);

      // Sendiing content chunk
      httpd_socket_send(this->_httpd_handle, this->fd_, ev.c_str(), ev.size(), 0);

      // Indicate end of chunk
      httpd_socket_send(this->_httpd_handle, this->fd_, CRLF_STR, CRLF_LEN, 0);
    }

  } // namespace web_server_idf
} // namespace esphome
