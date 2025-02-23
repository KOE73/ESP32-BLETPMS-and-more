#include "handler_static.h"

#include "json_util.h"
// #include "esphome/components/network/util.h"
// #include "esphome/core/application.h"
// #include "esphome/core/entity_base.h"
#include "helpers.h"
#include "esp_log.h"
// #include "esphome/core/util.h"

#include "web_url_match.hpp"

#include <cstdlib>

namespace yaidfws
{

  static const char *const TAG = "web_server";

#pragma region HandlerStaticUrl


  bool HandlerStaticUrl::canHandle(AsyncWebServerRequest *request)
  {

    if (request->url() == url_)
    {
      ESP_LOGI(TAG, "AsyncWebHandler_2::canHandle %s", request->url().c_str());
      return true;
    }

    return false;
  }

#pragma endregion

} // namespace yaidfws
