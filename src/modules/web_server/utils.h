#pragma once

#ifdef USE_ESP_IDF

#include <esp_http_server.h>
#include "helpers.h"

namespace web_server
{

    bool request_has_header(httpd_req_t *req, const char *name);
    std::optional<std::string> request_get_header(httpd_req_t *req, const char *name);
    std::optional<std::string> request_get_url_query(httpd_req_t *req);
    std::optional<std::string> query_key_value(const std::string &query_url, const std::string &key);

} // namespace web_server_idf

#endif // USE_ESP_IDF
