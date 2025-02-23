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

    class AsyncEventSourceResponse;

    class AsyncWebHandlerEventSource : public HandlerStaticUrl
    {
        friend class AsyncEventSourceResponse;
        using connect_handler_t = std::function<void(AsyncEventSourceResponse *)>;

    protected:
        std::set<AsyncEventSourceResponse *> _event_responses;
<<<<<<< HEAD:src/modules/web_server/web_handler_events.h
        connect_handler_t _on_connect{};

        SemaphoreHandle_t _sendMutex;
=======
        connect_handler_t on_connect_{};

        SemaphoreHandle_t sendMutex_;
>>>>>>> Web revert to main way. BLE:lib/yaidfws/src/handler_events.h
        
        void init();

    public:
        AsyncWebHandlerEventSource(std::string url);
        AsyncWebHandlerEventSource(IHandlerContainer &handlerContainer, std::string url);
        ~AsyncWebHandlerEventSource() override;

        bool canHandle(AsyncWebServerRequest *request) override
        {
            return request->method() == HTTP_GET && request->url() == this->_url;
        }

        void handleRequest(AsyncWebServerRequest *request) override;


<<<<<<< HEAD:src/modules/web_server/web_handler_events.h
        void onConnect(connect_handler_t cb) { this->_on_connect = std::move(cb); }
=======
        void onConnect(connect_handler_t cb) { this->on_connect_ = std::move(cb); }
>>>>>>> Web revert to main way. BLE:lib/yaidfws/src/handler_events.h

        /// @brief Send message to all event sessions
        /// @param message
        /// @param event
        /// @param id
        /// @param reconnect
        void send(const char *message, const char *event = nullptr, uint32_t id = 0, uint32_t reconnect = 0) const;

        size_t count() const { return this->_event_responses.size(); }
    };

    class AsyncEventSourceResponse
    {
        friend class AsyncWebHandlerEventSource;

    protected:
        AsyncWebHandlerEventSource *_eventSource;
        httpd_handle_t _httpd_handle{};
        int _sockfd{};

        AsyncEventSourceResponse(const AsyncWebServerRequest *request, AsyncWebHandlerEventSource *server);
        static void destroy(void *p);

    public:
        void send(const char *message, const char *event = nullptr, uint32_t id = 0, uint32_t reconnect = 0);
    };

} // namespace web_server_idf
