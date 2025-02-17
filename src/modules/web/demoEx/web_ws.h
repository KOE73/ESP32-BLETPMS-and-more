#pragma once

#include <esp_http_server.h>

#include <functional>
#include <map>
#include <set>
#include <string>
#include <vector>

// #include "optional.h"

#include "web_handler.h"

namespace web_server
{

    class AsyncWSSourceResponse;

    /// Добавить признак WS. В Addhandler добавить определение этого признака и ....
    class AsyncWebHandlerWSSource : public AsyncWebHandler
    {
        friend class AsyncWSSourceResponse;
        using connect_handler_t = std::function<void(AsyncWSSourceResponse *)>;

    protected:
        std::string url_;
        std::set<AsyncWSSourceResponse *> _ws_responses;
        connect_handler_t on_connect_{};

        const char *_key{"wwss"};

        SemaphoreHandle_t _sendMutex;

    public:
        AsyncWebHandlerWSSource(std::string url);
        ~AsyncWebHandlerWSSource() override;

        bool canHandle(AsyncWebServerRequest *request) override;

        void handleRequest(AsyncWebServerRequest *request) override;

        void onConnect(connect_handler_t cb) { this->on_connect_ = std::move(cb); }

        /// @brief Send message to all event sessions
        /// @param message
        /// @param event
        /// @param id
        /// @param reconnect
        void send(const char *message, const char *event = nullptr, uint32_t id = 0, uint32_t reconnect = 0) const;

        size_t count() const { return this->_ws_responses.size(); }
    };

    class AsyncWSSourceResponse
    {
        friend class AsyncWebHandlerWSSource;

    protected:
        AsyncWebHandlerWSSource *_eventSource;
        httpd_handle_t _httpd_handle{};
        int _sockfd{};

        AsyncWSSourceResponse(const AsyncWebServerRequest *request, AsyncWebHandlerWSSource *server);
        static void destroy(void *p);

    public:
        void send(const char *message, const char *event = nullptr, uint32_t id = 0, uint32_t reconnect = 0);
    };

} // namespace web_server_idf
