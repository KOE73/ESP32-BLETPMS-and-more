#pragma once

#include <esp_http_server.h>

#include <functional>
#include <map>
#include <set>
#include <string>
#include <vector>

// #include "optional.h"

#include "handler.h"

namespace web_server
{

    class AsyncWSSourceResponse;

    /// Добавить признак WS. В Addhandler добавить определение этого признака и ....
    class AsyncWebHandlerWSSource : public HandlerStaticUrl
    {
        friend class AsyncWSSourceResponse;
        using connect_handler_t = std::function<void(AsyncWSSourceResponse *)>;

    protected:
        std::set<AsyncWSSourceResponse *> _ws_responses;
        connect_handler_t _on_connect{};

        SemaphoreHandle_t _sendMutex;
        void init();

    public:
<<<<<<< HEAD:src/modules/web_server/web_handler_ws.h
=======

    
>>>>>>> Web revert to main way. BLE:lib/yaidfws/src/handler_ws.h
        AsyncWebHandlerWSSource(std::string url);
        AsyncWebHandlerWSSource(IHandlerContainer &handlerContainer, std::string url);
        ~AsyncWebHandlerWSSource() override;

        bool canHandle(AsyncWebServerRequest *request) override;

        void handleRequest(AsyncWebServerRequest *request) override;

        /// @brief 
        ///     Cannot call client->send()
        /// @param cb 
<<<<<<< HEAD:src/modules/web_server/web_handler_ws.h
        void onConnect(connect_handler_t cb) { this->_on_connect = std::move(cb); }
=======
        void onConnect(connect_handler_t cb) { this->on_connect_ = std::move(cb); }
>>>>>>> Web revert to main way. BLE:lib/yaidfws/src/handler_ws.h

        /// @brief Send message to all event sessions
        /// @param message
        /// @param event
        /// @param id
        /// @param reconnect
        void send(const char *message, uint32_t id = 0, uint32_t reconnect = 0) const;

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
        void send(const char *message, uint32_t id = 0, uint32_t reconnect = 0);
    };

} // namespace web_server_idf
