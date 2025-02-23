#pragma once

#include <esp_http_server.h>

#include <functional>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "web_request.h"

namespace yaidfws
{
    class HandlerBase;
    class IHandlerContainer
    {
    public:
        virtual void add_handler(HandlerBase *handler) = 0; // Чисто виртуальная функция
        virtual ~IHandlerContainer() {}
    };

    template <typename T>
    concept HandlerDerived = std::is_base_of_v<HandlerBase, T>;

    template <typename T>
    concept ContainerDerived = std::is_base_of_v<IHandlerContainer, T>;

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

    template <typename Derived>
    struct OptionsBase
    {
    };

    class HandlerBase
    {
    public:
        template <typename Derived>
        struct OptionsT : OptionsBase<Derived>
        {
            std::optional<IHandlerContainer *> handlerContainer_;
            Derived &setHandlerContainer(IHandlerContainer &hc) { return handlerContainer_ = &hc, static_cast<Derived &>(*this); }
        };
        //struct Options : OptionsT<Options>{};
        //using Options = OptionsT<Options>;

        explicit HandlerBase(const OptionsT<> &opts = Options{})
        {
            
            if (opts.handlerContainer_.has_value() && opts.handlerContainer_.value())
            {
                opts.handlerContainer_.value()->add_handler(this);
            }
        }

        // HandlerBase() {}
        // HandlerBase(IHandlerContainer &handlerContainer) { handlerContainer.add_handler(this); }
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
        std::string url_;

    public:
        template <typename Derived>
        struct OptionsT : HandlerBase::OptionsT<Derived>
        {
            std::string url_;
            explicit OptionsT(std::string url) : url_(std::move(url)) {}
            // setHandlerContainer унаследован от HandlerBase::OptionsT
        };
        struct Options : OptionsT<Options>{};
        //using Options = OptionsT<Options>;

        explicit HandlerStaticUrl(const Options &opts)
            : HandlerBase(opts), url_(std::move(opts.url_)) {}

        // HandlerStaticUrl(std::string url) : url_(std::move(url)) {};
        // HandlerStaticUrl(IHandlerContainer &handlerContainer, std::string url) : HandlerBase(handlerContainer), url_(std::move(url)) {};
    };

#pragma region deduction guides

    // template <typename T>
    // concept HandlerDerived = std::is_base_of_v<HandlerBase, T>;
    //
    // template <typename T>
    // concept ContainerDerived = std::is_base_of_v<IHandlerContainer, T>;

    // Шаблонная функция opt
    template <HandlerDerived HandlerType>
    typename HandlerType::Options opt(std::string url = "")
    {
        typename HandlerType::Options opts(url);
        return opts;
    }

    template <HandlerDerived HandlerType, ContainerDerived ContainerType>
    typename HandlerType::Options opt(ContainerType &hc)
    {
        typename HandlerType::Options opts;
        opts.setHandlerContainer(hc);
        return opts;
    }

    template <HandlerDerived HandlerType, ContainerDerived ContainerType>
    typename HandlerType::Options opt(ContainerType &hc, std::string url = "")
    {
        typename HandlerType::Options opts(url);
        opts.setHandlerContainer(hc);
        return opts;
    }
#pragma endregion

    //// Вспомогательная функция для IHandlerContainer
    // HandlerBase::Options opt(IHandlerContainer &hc)
    //{
    //     HandlerBase::Options opts;
    //     opts.setHandlerContainer(hc);
    //     return opts;
    // }
    //
    // HandlerStaticUrl::Options opt(IHandlerContainer &hc, std::string url = "")
    //{
    //    HandlerStaticUrl::Options opts(url);
    //    opts.setHandlerContainer(hc);
    //    return opts;
    //}
} // namespace yaidfws
