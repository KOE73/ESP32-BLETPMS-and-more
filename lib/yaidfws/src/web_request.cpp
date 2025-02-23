

#include <cstdarg>

// #include "esphome/core/log.h"
// #include "esphome/core/helpers.h"

#include "esp_log.h"

#include "esp_tls_crypto.h"

#include "utils.h"
#include "web_server_idf.h"

static const char *TAG_WEB2_SERVER = "WEB2_SERVER";
#define LOG_WEB2_COLOR LOG_ANSI_COLOR_BOLD_BACKGROUND(LOG_COLOR_BLUE, LOG_ANSI_COLOR_BG_CYAN)

namespace yaidfws
{

#ifndef HTTPD_409
#define HTTPD_409 "409 Conflict"
#endif

#define CRLF_STR "\r\n"
#define CRLF_LEN (sizeof(CRLF_STR) - 1)

    static const char *const TAG = "web_server_idf";

    AsyncWebServerRequest::~AsyncWebServerRequest()
    {
        delete this->rsp_;
        for (const auto &pair : this->params_)
        {
            delete pair.second; // NOLINT(cppcoreguidelines-owning-memory)
        }
    }

    bool AsyncWebServerRequest::hasHeader(const char *name) const
    {
        // return request_has_header(getHttpdReq(), name);
        return httpd_req_get_hdr_value_len(httpd_req_, name);
    }

    std::optional<std::string> AsyncWebServerRequest::get_header(const char *name) const
    {
        // return request_get_header(httpd_req_, name);
        size_t len = httpd_req_get_hdr_value_len(httpd_req_, name);
        if (len == 0)
        {
            return {};
        }

        std::string str;
        str.resize(len);

        auto res = httpd_req_get_hdr_value_str(httpd_req_, name, &str[0], len + 1);
        if (res != ESP_OK)
        {
            return {};
        }

        return {str};
    }

    // std::string AsyncWebServerRequest::url() const
    //{
    //     auto *str = strchr(this->httpd_req_->uri, '?');
    //     if (str == nullptr)
    //     {
    //         return this->httpd_req_->uri;
    //     }
    //     return std::string(this->httpd_req_->uri, str - this->httpd_req_->uri);
    // }

    std::string AsyncWebServerRequest::host() const { return this->get_header("Host").value(); }

    void AsyncWebServerRequest::send(AsyncWebServerResponse *response)
    {
        httpd_resp_send(getHttpdReq(), response->get_content_data(), response->get_content_size());
    }

    void AsyncWebServerRequest::send(int code, const char *content_type, const char *content)
    {
        this->init_response_(nullptr, code, content_type);
        if (content)
        {
            httpd_resp_send(getHttpdReq(), content, HTTPD_RESP_USE_STRLEN);
        }
        else
        {
            httpd_resp_send(getHttpdReq(), nullptr, 0);
        }
    }

    void AsyncWebServerRequest::redirect(const std::string &url)
    {
        httpd_resp_set_status(getHttpdReq(), "302 Found");
        httpd_resp_set_hdr(getHttpdReq(), "Location", url.c_str());
        httpd_resp_send(getHttpdReq(), nullptr, 0);
    }

    void AsyncWebServerRequest::init_response_(AsyncWebServerResponse *rsp, int code, const char *content_type)
    {
        httpd_resp_set_status(getHttpdReq(), code == 200   ? HTTPD_200
                                             : code == 404 ? HTTPD_404
                                             : code == 409 ? HTTPD_409
                                                           : std::to_string(code).c_str());

        if (content_type && *content_type)
        {
            httpd_resp_set_type(getHttpdReq(), content_type);
        }
        httpd_resp_set_hdr(getHttpdReq(), "Accept-Ranges", "none");

        for (const auto &pair : DefaultHeaders::Instance().headers_)
        {
            httpd_resp_set_hdr(getHttpdReq(), pair.first.c_str(), pair.second.c_str());
        }

        delete this->rsp_;
        this->rsp_ = rsp;
    }

    bool AsyncWebServerRequest::authenticate(const char *username, const char *password) const
    {
        if (username == nullptr || password == nullptr || *username == 0)
        {
            return true;
        }
        auto auth = this->get_header("Authorization");
        if (!auth.has_value())
        {
            return false;
        }

        auto *auth_str = auth.value().c_str();

        const auto auth_prefix_len = sizeof("Basic ") - 1;
        if (strncmp("Basic ", auth_str, auth_prefix_len) != 0)
        {
            ESP_LOGW(TAG, "Only Basic authorization supported yet");
            return false;
        }

        std::string user_info;
        user_info += username;
        user_info += ':';
        user_info += password;

        size_t n = 0, out;
        esp_crypto_base64_encode(nullptr, 0, &n, reinterpret_cast<const uint8_t *>(user_info.c_str()), user_info.size());

        auto digest = std::unique_ptr<char[]>(new char[n + 1]);
        esp_crypto_base64_encode(reinterpret_cast<uint8_t *>(digest.get()), n, &out,
                                 reinterpret_cast<const uint8_t *>(user_info.c_str()), user_info.size());

        return strncmp(digest.get(), auth_str + auth_prefix_len, auth.value().size() - auth_prefix_len) == 0;
    }

    void AsyncWebServerRequest::requestAuthentication(const char *realm) const
    {
        httpd_resp_set_hdr(getHttpdReq(), "Connection", "keep-alive");
        // TODO !!!!
        // auto auth_val = str_sprintf("Basic realm=\"%s\"", realm ? realm : "Login Required");
        // httpd_resp_set_hdr(getHttpdReq(), "WWW-Authenticate", auth_val.c_str());
        httpd_resp_send_err(getHttpdReq(), HTTPD_401_UNAUTHORIZED, nullptr);
    }

    AsyncWebParameter *AsyncWebServerRequest::getParam(const std::string &name)
    {
        auto find = this->params_.find(name);
        if (find != this->params_.end())
        {
            return find->second;
        }

        std::optional<std::string> val = query_key_value(this->post_query_, name);
        if (!val.has_value())
        {
            auto url_query = request_get_url_query(getHttpdReq());
            if (url_query.has_value())
            {
                val = query_key_value(url_query.value(), name);
            }
        }

        AsyncWebParameter *param = nullptr;
        if (val.has_value())
        {
            param = new AsyncWebParameter(val.value()); // NOLINT(cppcoreguidelines-owning-memory)
        }
        this->params_.insert({name, param});
        return param;
    }

#pragma region From Utils
    size_t parse_hex(const char *str, size_t length, uint8_t *data, size_t count)
    {
        uint8_t val;
        size_t chars = std::min(length, 2 * count);
        for (size_t i = 2 * count - chars; i < 2 * count; i++, str++)
        {
            if (*str >= '0' && *str <= '9')
            {
                val = *str - '0';
            }
            else if (*str >= 'A' && *str <= 'F')
            {
                val = 10 + (*str - 'A');
            }
            else if (*str >= 'a' && *str <= 'f')
            {
                val = 10 + (*str - 'a');
            }
            else
            {
                return 0;
            }
            data[i >> 1] = !(i & 1) ? val << 4 : data[i >> 1] | val;
        }
        return chars;
    }

    void url_decode(char *str)
    {
        char *ptr = str, buf;
        for (; *str; str++, ptr++)
        {
            if (*str == '%')
            {
                str++;
                if (parse_hex(str, 2, reinterpret_cast<uint8_t *>(&buf), 1) == 2)
                {
                    *ptr = buf;
                    str++;
                }
                else
                {
                    str--;
                    *ptr = *str;
                }
            }
            else if (*str == '+')
            {
                *ptr = ' ';
            }
            else
            {
                *ptr = *str;
            }
        }
        *ptr = *str;
    }

    bool AsyncWebServerRequest::request_has_header(httpd_req_t *req, const char *name) { return httpd_req_get_hdr_value_len(req, name); }

    std::optional<std::string> AsyncWebServerRequest::request_get_header(httpd_req_t *req, const char *name)
    {
        size_t len = httpd_req_get_hdr_value_len(req, name);
        if (len == 0)
        {
            return {};
        }

        std::string str;
        str.resize(len);

        auto res = httpd_req_get_hdr_value_str(req, name, &str[0], len + 1);
        if (res != ESP_OK)
        {
            return {};
        }

        return {str};
    }

    std::optional<std::string> AsyncWebServerRequest::request_get_url_query(httpd_req_t *req)
    {
        auto len = httpd_req_get_url_query_len(req);
        if (len == 0)
        {
            return {};
        }

        std::string str;
        str.resize(len);

        auto res = httpd_req_get_url_query_str(req, &str[0], len + 1);
        if (res != ESP_OK)
        {
            ESP_LOGW(TAG, "Can't get query for request: %s", esp_err_to_name(res));
            return {};
        }

        return {str};
    }

    std::optional<std::string> AsyncWebServerRequest::query_key_value(const std::string &query_url, const std::string &key)
    {
        if (query_url.empty())
        {
            return {};
        }

        auto val = std::unique_ptr<char[]>(new char[query_url.size()]);
        if (!val)
        {
            ESP_LOGE(TAG, "Not enough memory to the query key value");
            return {};
        }

        if (httpd_query_key_value(query_url.c_str(), key.c_str(), val.get(), query_url.size()) != ESP_OK)
        {
            return {};
        }

        url_decode(val.get());
        return {val.get()};
    }
#pragma endregion

} // namespace yaidfws