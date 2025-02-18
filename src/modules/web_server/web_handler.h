#pragma once

#include <esp_http_server.h>

#include <functional>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "web_request.h"

namespace web_server
{
    class HandlerBase;
    class IHandlerContainer
    {
    public:
        virtual void add_handler(HandlerBase *handler) = 0; // Чисто виртуальная функция
        virtual ~IHandlerContainer() {}
    };

    // class IHandler
    //{
    // public:
    //     virtual ~IHandler() {}
    //
    //    virtual bool isRequestHandlerTrivial() { return true; }
    //
    //    virtual bool canHandle(AsyncWebServerRequest *request) { return false; }
    //
    //    virtual void handleRequest(AsyncWebServerRequest *request) {}
    //    virtual void handleUpload(AsyncWebServerRequest *request, const std::string &filename, size_t index, uint8_t *data,
    //                              size_t len, bool final) {}
    //    virtual void handleBody(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {}
    //
    //    /// @brief  Custom handlers can override this method to define whether they need to wait for the request body.
    //    /// @return
    //    /// true - for simple GET requests that do not require body processing.
    //    ///
    //    /// false - for requests that contain a body (e.g., POST, PUT, file uploads).
    //    ///
    //};

    class HandlerBase
    {
    public:
        HandlerBase() {}
        HandlerBase(IHandlerContainer &handlerContainer) { handlerContainer.add_handler(this); }
        virtual ~HandlerBase() {}

        /// @brief  Custom handlers can override this method to define whether they need to wait for the request body.
        /// @return
        /// true - for simple GET requests that do not require body processing.
        ///
        /// false - for requests that contain a body (e.g., POST, PUT, file uploads).
        ///
        virtual bool isRequestHandlerTrivial() { return true; }
        virtual bool canHandle(AsyncWebServerRequest *request) { return false; }
        virtual void handleRequest(AsyncWebServerRequest *request) {}
        virtual void handleUpload(AsyncWebServerRequest *request, const std::string &filename, size_t index, uint8_t *data,
                                  size_t len, bool final) {}
        virtual void handleBody(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {}
    };

    class HandlerStaticUrl : public HandlerBase // public Controller, public Component,
    {
    protected:
        std::string _url;

    public:
        HandlerStaticUrl(std::string url) : _url(std::move(url)) {};
        HandlerStaticUrl(IHandlerContainer &handlerContainer, std::string url) : HandlerBase(handlerContainer), _url(std::move(url)) {};
    };
} // namespace web_server_idf
