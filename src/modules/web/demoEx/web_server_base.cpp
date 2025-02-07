#include "web_server_base.h"
#ifdef USE_NETWORK
// #include "esphome/core/log.h"
// #include "esphome/core/application.h"
// #include "esphome/core/helpers.h"

namespace esphome
{
  namespace web_server_base
  {

    static const char *const TAG = "web_server_base";

    void WebServerBaseComponent::add_handler(AsyncWebHandler *handler)
    {
      // remove all handlers

      if (!credentials_.username.empty())
      {
        handler = new AuthMiddlewareHandler(handler, &credentials_);
      }
      this->handlers_.push_back(handler);
      if (this->_IDFWebServer != nullptr)
      {
        this->_IDFWebServer->addHandler(handler);
      }
    }

 
    void WebServerBaseComponent::add_ota_handler()
    {
#ifdef USE_ARDUINO
      this->add_handler(new OTARequestHandler(this)); // NOLINT
#endif
    }
    float WebServerBaseComponent::get_setup_priority() const
    {
      // Before WiFi (captive portal)
      // !!! return setup_priority::WIFI + 2.0f;
      return 20.0f;
    }

  } // namespace web_server_base
} // namespace esphome
#endif
