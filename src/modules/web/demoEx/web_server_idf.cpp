#include <cstdarg>

// #include "esphome/core/log.h"
// #include "esphome/core/helpers.h"

#include "esp_log.h"

#include "esp_tls_crypto.h"

#include "utils.h"
#include "web_server_idf.h"

static const char *TAG_WEB2_SERVER = "WEB2_SERVER";
#define LOG_WEB2_COLOR LOG_ANSI_COLOR_BOLD_BACKGROUND(LOG_COLOR_BLUE, LOG_ANSI_COLOR_BG_CYAN)

namespace web_server
{

#ifndef HTTPD_409
#define HTTPD_409 "409 Conflict"
#endif

#define CRLF_STR "\r\n"
#define CRLF_LEN (sizeof(CRLF_STR) - 1)

  static const char *const TAG = "web_server_idf";

  void IDFWebServer::end()
  {
    if (this->_httpd_handle)
    {
      httpd_stop(this->_httpd_handle);
      this->_httpd_handle = nullptr;
    }
  }

  void IDFWebServer::begin()
  {
    ESP_LOGI(TAG_WEB2_SERVER, LOG_WEB2_COLOR "Web2 Server Begin 1" LOG_ANSI_COLOR_RESET);

    if (this->_httpd_handle)
    {
      this->end();
    }
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = this->port_;
    config.uri_match_fn = [](const char * /*unused*/, const char * /*unused*/, size_t /*unused*/)
    {
      return true;
    };

    if (httpd_start(&this->_httpd_handle, &config) == ESP_OK)
    {
      ESP_LOGI(TAG_WEB2_SERVER, LOG_WEB2_COLOR "Web2 Server Begin 2" LOG_ANSI_COLOR_RESET);

      const httpd_uri_t handler_get = {
          .uri = "",
          .method = HTTP_GET,
          .handler = IDFWebServer::request_get_handler,
          .user_ctx = this,
#ifdef CONFIG_HTTPD_WS_SUPPORT
          .is_websocket = true
#endif
      };
      httpd_register_uri_handler(this->_httpd_handle, &handler_get);

      const httpd_uri_t handler_post = {
          .uri = "",
          .method = HTTP_POST,
          .handler = IDFWebServer::request_post_handler,
          .user_ctx = this,
      };
      httpd_register_uri_handler(this->_httpd_handle, &handler_post);

      const httpd_uri_t handler_options = {
          .uri = "",
          .method = HTTP_OPTIONS,
          .handler = IDFWebServer::request_get_handler,
          .user_ctx = this,
      };
      httpd_register_uri_handler(this->_httpd_handle, &handler_options);
    }
  }

#pragma region Handlers from C world to convert to C++ world

  esp_err_t IDFWebServer::request_get_handler(httpd_req_t *r)
  {
    // ESP_LOGVV(TAG, "Enter AsyncWebServer::request_handler. method=%u, uri=%s", r->method, r->uri);
    ESP_LOGI(TAG_WEB2_SERVER, LOG_WEB2_COLOR "Enter AsyncWebServer::request_handler. method=%u, uri=%s", r->method, r->uri);
    AsyncWebServerRequest req(r);
    return static_cast<IDFWebServer *>(r->user_ctx)->request_handler_(&req);
  }

  esp_err_t IDFWebServer::request_post_handler(httpd_req_t *r)
  {
    // ESP_LOGVV(TAG, "Enter AsyncWebServer::request_post_handler. uri=%s", r->uri);
    ESP_LOGI(TAG_WEB2_SERVER, LOG_WEB2_COLOR "Enter AsyncWebServer::request_post_handler. uri=%s", r->uri);

    auto content_type = request_get_header(r, "Content-Type");
    if (content_type.has_value() && *content_type != "application/x-www-form-urlencoded")
    {
      ESP_LOGW(TAG, "Only application/x-www-form-urlencoded supported for POST request");
      // fallback to get handler to support backward compatibility
      return IDFWebServer::request_get_handler(r);
    }

    if (!request_has_header(r, "Content-Length"))
    {
      ESP_LOGW(TAG, "Content length is requred for post: %s", r->uri);
      httpd_resp_send_err(r, HTTPD_411_LENGTH_REQUIRED, nullptr);
      return ESP_OK;
    }

    if (r->content_len > HTTPD_MAX_REQ_HDR_LEN)
    {
      ESP_LOGW(TAG, "Request size is to big: %zu", r->content_len);
      httpd_resp_send_err(r, HTTPD_400_BAD_REQUEST, nullptr);
      return ESP_FAIL;
    }

    std::string post_query;
    if (r->content_len > 0)
    {
      post_query.resize(r->content_len);
      const int ret = httpd_req_recv(r, &post_query[0], r->content_len + 1);
      if (ret <= 0)
      { // 0 return value indicates connection closed
        if (ret == HTTPD_SOCK_ERR_TIMEOUT)
        {
          httpd_resp_send_err(r, HTTPD_408_REQ_TIMEOUT, nullptr);
          return ESP_ERR_TIMEOUT;
        }
        httpd_resp_send_err(r, HTTPD_400_BAD_REQUEST, nullptr);
        return ESP_FAIL;
      }
    }

    AsyncWebServerRequest req(r, std::move(post_query));
    return static_cast<IDFWebServer *>(r->user_ctx)->request_handler_(&req);
  }

#pragma endregion

  // Метод обработки запросв в мире классов
  esp_err_t IDFWebServer::request_handler_(AsyncWebServerRequest *request) const
  {
    ESP_LOGI(TAG_WEB2_SERVER, LOG_WEB2_COLOR "IDFWebServer::request_handler_. count=%i" LOG_ANSI_COLOR_RESET, handlers_.size());

    for (auto *handler : this->handlers_)
    {
      ESP_LOGI(TAG_WEB2_SERVER, LOG_WEB2_COLOR "IDFWebServer::request_handler_. canHandle=%i" LOG_ANSI_COLOR_RESET, (int)handler->canHandle(request));

      if (handler->canHandle(request))
      {
        // At now process only basic requests.
        // OTA requires multipart request support and handleUpload for it
        handler->handleRequest(request);
        return ESP_OK;
      }
    }
    if (this->on_not_found_)
    {
      this->on_not_found_(request);
      return ESP_OK;
    }
    return ESP_ERR_NOT_FOUND;
  }

} // namespace web_server_idf
