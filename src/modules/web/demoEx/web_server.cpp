#include "web_server.h"

#ifdef USE_WEBSERVER
#include "json_util.h"
// #include "esphome/components/network/util.h"
// #include "esphome/core/application.h"
// #include "esphome/core/entity_base.h"
#include "helpers.h"
#include "esp_log.h"
// #include "esphome/core/util.h"

#include "web_events.h"

#ifdef USE_ARDUINO
#include "StreamString.h"
#endif

#include <cstdlib>


namespace esphome
{
  namespace web_server
  {

    static const char *const TAG = "web_server";




  } // namespace web_server
} // namespace esphome
#endif
