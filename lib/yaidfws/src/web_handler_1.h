#pragma once

#include <map>
#include <vector>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <deque>

// #include "web_server_container.h"
#include "web_handler.h"

namespace yaidfws
{

  /// @brief For static text
  class HandlerStaticUriText : public HandlerStaticUrl
  {
  private:
    const char *_text;
    const ssize_t _text_len;

  protected:
    bool canHandle(AsyncWebServerRequest *request) override;
    void handleRequest(AsyncWebServerRequest *request) override;

  public:
    template <typename Derived>
    struct OptionsT : HandlerStaticUrl::OptionsT<Derived>
    {
      std::optional<const char *> text;
      std::optional<ssize_t> text_len;
      explicit OptionsT(std::string u) : HandlerStaticUrl::OptionsT(std::move(u)) {}
      Derived &setText(const char *t) { return text = t, *this; }
      Derived &setTextLen(ssize_t len) { return text_len = len, *this; }
    };
    using Options = OptionsT<Options>;

    explicit HandlerStaticUriText(const Options &opts)
        : HandlerStaticUrl(opts),
          _text(opts.text.value_or(nullptr)),
          _text_len(opts.text_len.value_or(-1)) {}

    // HandlerStaticUriText(std::string url);
    // HandlerStaticUriText(std::string url, const char *text);
    // HandlerStaticUriText(std::string url, const char *text, ssize_t text_len);
    // HandlerStaticUriText(IHandlerContainer &handlerContainer, std::string url);
    // HandlerStaticUriText(IHandlerContainer &handlerContainer, std::string url, const char *text);
    // HandlerStaticUriText(IHandlerContainer &handlerContainer, std::string url, const char *text, ssize_t text_len);
  };

  //// Вспомогательная функция для IHandlerContainer
  // HandlerStaticUriText::Options opt(IHandlerContainer &hc, std::string url = "")
  //{
  //   HandlerStaticUriText::Options opts(url);
  //   opts.setHandlerContainer(hc);
  //   return opts;
  // }

  /// @brief For binary and may be gziped
  class HandlerStaticUriBin : public HandlerStaticUrl
  {
  private:
    const uint8_t *_buf;
    const ssize_t _buf_len;
    const bool _is_gzip{false};

  protected:
    /// Override the web handler's canHandle method.
    bool canHandle(AsyncWebServerRequest *request) override;
    /// Override the web handler's handleRequest method.
    void handleRequest(AsyncWebServerRequest *request) override;

  public:
    template <typename Derived>
    struct OptionsT : HandlerStaticUrl::OptionsT<Derived>
    {
      const uint8_t *buf_;          // Обязательный
      ssize_t buf_len_;             // Обязательный
      std::optional<bool> is_gzip_; // Опциональный

      // Конструктор с обязательными параметрами
      OptionsT(std::string url, const uint8_t *buf, ssize_t buf_len)
          : HandlerStaticUrl::OptionsT(std::move(url)), buf_(buf), buf_len_(buf_len) {}

      // Методы для опциональных параметров
      // Options &setBuf(const uint8_t *buf, ssize_t buf_len) { return buf_ = buf, buf_len_ = buf_len, *this; }
      Options &setIsGzip(bool gzip) { return is_gzip_ = gzip, *this; }
      Options &setHandlerContainer(IHandlerContainer &hc) { return HandlerStaticUrl::Options::setHandlerContainer(hc), *this; }
    };

    using Options = OptionsT<Options>;

    explicit HandlerStaticUriBin(const Options &opts)
        : HandlerStaticUrl(opts),
          _buf(opts.buf_),
          _buf_len(opts.buf_len_),
          _is_gzip(opts.is_gzip_.value_or(false)) {}

    // HandlerStaticUriBin(std::string url, const uint8_t *buf, ssize_t buf_len, bool is_gzip = false);
    // HandlerStaticUriBin(IHandlerContainer &handlerContainer, std::string url, const uint8_t *buf, ssize_t buf_len, bool is_gzip = false);
  };

  template <HandlerDerived HandlerType>
  typename HandlerType::Options opt(std::string url, const uint8_t *buf, ssize_t buf_len)
  {
    typename HandlerType::Options opts(url, buf, buf_len);
    return opts;
  }

} // namespace yaidfws
