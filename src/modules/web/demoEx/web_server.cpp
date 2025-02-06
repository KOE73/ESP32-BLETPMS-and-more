#include "web_server.h"

#ifdef USE_WEBSERVER
#include "json_util.h"
// #include "esphome/components/network/util.h"
// #include "esphome/core/application.h"
// #include "esphome/core/entity_base.h"
#include "helpers.h"
#include "esp_log.h"
// #include "esphome/core/util.h"

#ifdef USE_ARDUINO
#include "StreamString.h"
#endif

#include <cstdlib>

#ifdef USE_LOGGER
#include "esphome/components/logger/logger.h"
#endif

#ifdef USE_CLIMATE
#include "esphome/components/climate/climate.h"
#endif

#ifdef USE_WEBSERVER_LOCAL
#if USE_WEBSERVER_VERSION == 2
#include "server_index_v2.h"
#elif USE_WEBSERVER_VERSION == 3
#include "server_index_v3.h"
#endif
#endif

namespace esphome
{
  namespace web_server
  {

    static const char *const TAG = "web_server";

#ifdef USE_WEBSERVER_PRIVATE_NETWORK_ACCESS
    static const char *const HEADER_PNA_NAME = "Private-Network-Access-Name";
    static const char *const HEADER_PNA_ID = "Private-Network-Access-ID";
    static const char *const HEADER_CORS_REQ_PNA = "Access-Control-Request-Private-Network";
    static const char *const HEADER_CORS_ALLOW_PNA = "Access-Control-Allow-Private-Network";
#endif

    /// @brief Разбор проинятого url. В перспективе переделать в класс?
    /// @param url
    /// @param only_domain
    /// @return Структура с данными, что там в строке
    UrlMatch match_url(const std::string &url, bool only_domain = false)
    {
      UrlMatch match;
      match.valid = false;
      size_t domain_end = url.find('/', 1);
      if (domain_end == std::string::npos)
        return match;
      match.domain = url.substr(1, domain_end - 1);
      if (only_domain)
      {
        match.valid = true;
        return match;
      }
      if (url.length() == domain_end - 1)
        return match;
      size_t id_begin = domain_end + 1;
      size_t id_end = url.find('/', id_begin);
      match.valid = true;
      if (id_end == std::string::npos)
      {
        match.id = url.substr(id_begin, url.length() - id_begin);
        return match;
      }
      match.id = url.substr(id_begin, id_end - id_begin);
      size_t method_begin = id_end + 1;
      match.method = url.substr(method_begin, url.length() - method_begin);
      return match;
    }

    WebServerControllerComponent::WebServerControllerComponent(web_server_base::WebServerBaseComponent *base)
        : _webServerBaseComponent(base),
          entities_iterator_(ListEntitiesIterator(this))
    {
#ifdef USE_ESP32
      to_schedule_lock_ = xSemaphoreCreateMutex();
#endif
    }

#ifdef USE_WEBSERVER_CSS_INCLUDE
    void WebServer::set_css_include(const char *css_include) { this->css_include_ = css_include; }
#endif
#ifdef USE_WEBSERVER_JS_INCLUDE
    void WebServer::set_js_include(const char *js_include) { this->js_include_ = js_include; }
#endif

    std::string WebServerControllerComponent::get_config_json()
    {
      return json::build_json([this](JsonObject root)
                              {
    //root["title"] = App.get_friendly_name().empty() ? App.get_name() : App.get_friendly_name();
    //root["comment"] = App.get_comment();
    root["ota"] = this->allow_ota_;
    root["log"] = this->expose_log_;
    root["lang"] = "en"; });
    }

    void WebServerControllerComponent::setup()
    {
      // ESP_LOGCONFIG(TAG, "Setting up web server...");
      ESP_LOGI(TAG, "Setting up web server...");

      // this->setup_controller(this->include_internal_);

      this->_webServerBaseComponent->init();

      this->events_.onConnect([this](AsyncEventSourceClient *client)
                              {
                                // Configure reconnect timeout and send config
                                client->send(this->get_config_json().c_str(), "ping", 1000 /*millis()*/, 30000);

                                // for (auto &group : this->sorting_groups_) {
                                //   client->send(json::build_json([group](JsonObject root) {
                                //                  root["name"] = group.second.name;
                                //                  root["sorting_weight"] = group.second.weight;
                                //                }).c_str(),
                                //                "sorting_group");
                                // }

                                // this->entities_iterator_.begin(this->include_internal_);
                              });

#ifdef USE_LOGGER
      if (logger::global_logger != nullptr && this->expose_log_)
      {
        logger::global_logger->add_on_log_callback(
            [this](int level, const char *tag, const char *message)
            { this->events_.send(message, "log", millis()); });
      }
#endif
      this->_webServerBaseComponent->add_handler(&this->events_);

      // перенес в WebServerControllerComponent
      this->_webServerBaseComponent->add_handler(&_main_handler);

      if (this->allow_ota_)
        this->_webServerBaseComponent->add_ota_handler();

      // this->set_interval(10000, [this]()
      //                    { this->events_.send("", "ping", 1000 /*millis()*/, 30000); });
    }

    void WebServerControllerComponent::loop()
    {
#ifdef USE_ESP32
      if (xSemaphoreTake(this->to_schedule_lock_, 0L))
      {
        std::function<void()> fn;
        if (!to_schedule_.empty())
        {
          // scheduler execute things out of order which may lead to incorrect state
          // this->defer(std::move(to_schedule_.front()));
          // let's execute it directly from the loop
          fn = std::move(to_schedule_.front());
          to_schedule_.pop_front();
        }
        xSemaphoreGive(this->to_schedule_lock_);
        if (fn)
        {
          fn();
        }
      }
#endif
      // this->entities_iterator_.advance();
    }

    void WebServerControllerComponent::dump_config()
    {
      // ESP_LOGCONFIG(TAG, "Web Server:");
      // ESP_LOGCONFIG(TAG, "  Address: %s:%u", network::get_use_address().c_str(), this->_webServerBaseComponent->get_port());
    }

    float WebServerControllerComponent::get_setup_priority() const
    {
      // return setup_priority::WIFI - 1.0f;
      return 5.0f;
    }

#ifdef USE_WEBSERVER_PRIVATE_NETWORK_ACCESS
    void WebServer::handle_pna_cors_request(AsyncWebServerRequest *request)
    {
      AsyncWebServerResponse *response = request->beginResponse(200, "");
      response->addHeader(HEADER_CORS_ALLOW_PNA, "true");
      response->addHeader(HEADER_PNA_NAME, App.get_name().c_str());
      std::string mac = get_mac_address_pretty();
      response->addHeader(HEADER_PNA_ID, mac.c_str());
      request->send(response);
    }
#endif

#ifdef USE_WEBSERVER_CSS_INCLUDE
    void WebServer::handle_css_request(AsyncWebServerRequest *request)
    {
      AsyncWebServerResponse *response =
          request->beginResponse_P(200, "text/css", ESPHOME_WEBSERVER_CSS_INCLUDE, ESPHOME_WEBSERVER_CSS_INCLUDE_SIZE);
      response->addHeader("Content-Encoding", "gzip");
      request->send(response);
    }
#endif

#ifdef USE_WEBSERVER_JS_INCLUDE
    void WebServer::handle_js_request(AsyncWebServerRequest *request)
    {
      AsyncWebServerResponse *response =
          request->beginResponse_P(200, "text/javascript", ESPHOME_WEBSERVER_JS_INCLUDE, ESPHOME_WEBSERVER_JS_INCLUDE_SIZE);
      response->addHeader("Content-Encoding", "gzip");
      request->send(response);
    }
#endif

#define set_json_id(root, obj, sensor, start_config)                      \
  (root)["id"] = sensor;                                                  \
  if (((start_config) == DETAIL_ALL))                                     \
  {                                                                       \
    (root)["name"] = (obj)->get_name();                                   \
    (root)["icon"] = (obj)->get_icon();                                   \
    (root)["entity_category"] = (obj)->get_entity_category();             \
    if ((obj)->is_disabled_by_default())                                  \
      (root)["is_disabled_by_default"] = (obj)->is_disabled_by_default(); \
  }

#define set_json_value(root, obj, sensor, value, start_config) \
  set_json_id((root), (obj), sensor, start_config);            \
  (root)["value"] = value;

#define set_json_icon_state_value(root, obj, sensor, state, value, start_config) \
  set_json_value(root, obj, sensor, value, start_config);                        \
  (root)["state"] = state;

#ifdef USE_SENSOR
    void WebServer::on_sensor_update(sensor::Sensor *obj, float state)
    {
      if (this->events_.count() == 0)
        return;
      this->events_.send(this->sensor_json(obj, state, DETAIL_STATE).c_str(), "state");
    }
    void WebServer::handle_sensor_request(AsyncWebServerRequest *request, const UrlMatch &match)
    {
      for (sensor::Sensor *obj : App.get_sensors())
      {
        if (obj->get_object_id() != match.id)
          continue;
        if (request->method() == HTTP_GET && match.method.empty())
        {
          auto detail = DETAIL_STATE;
          auto *param = request->getParam("detail");
          if (param && param->value() == "all")
          {
            detail = DETAIL_ALL;
          }
          std::string data = this->sensor_json(obj, obj->state, detail);
          request->send(200, "application/json", data.c_str());
          return;
        }
      }
      request->send(404);
    }
    std::string WebServer::sensor_json(sensor::Sensor *obj, float value, JsonDetail start_config)
    {
      return json::build_json([this, obj, value, start_config](JsonObject root)
                              {
    std::string state;
    if (std::isnan(value)) {
      state = "NA";
    } else {
      state = value_accuracy_to_string(value, obj->get_accuracy_decimals());
      if (!obj->get_unit_of_measurement().empty())
        state += " " + obj->get_unit_of_measurement();
    }
    set_json_icon_state_value(root, obj, "sensor-" + obj->get_object_id(), state, value, start_config);
    if (start_config == DETAIL_ALL) {
      if (this->sorting_entitys_.find(obj) != this->sorting_entitys_.end()) {
        root["sorting_weight"] = this->sorting_entitys_[obj].weight;
        if (this->sorting_groups_.find(this->sorting_entitys_[obj].group_id) != this->sorting_groups_.end()) {
          root["sorting_group"] = this->sorting_groups_[this->sorting_entitys_[obj].group_id].name;
        }
      }
      if (!obj->get_unit_of_measurement().empty())
        root["uom"] = obj->get_unit_of_measurement();
    } });
    }
#endif

#ifdef USE_TEXT_SENSOR
    void WebServer::on_text_sensor_update(text_sensor::TextSensor *obj, const std::string &state)
    {
      if (this->events_.count() == 0)
        return;
      this->events_.send(this->text_sensor_json(obj, state, DETAIL_STATE).c_str(), "state");
    }
    void WebServer::handle_text_sensor_request(AsyncWebServerRequest *request, const UrlMatch &match)
    {
      for (text_sensor::TextSensor *obj : App.get_text_sensors())
      {
        if (obj->get_object_id() != match.id)
          continue;
        if (request->method() == HTTP_GET && match.method.empty())
        {
          auto detail = DETAIL_STATE;
          auto *param = request->getParam("detail");
          if (param && param->value() == "all")
          {
            detail = DETAIL_ALL;
          }
          std::string data = this->text_sensor_json(obj, obj->state, detail);
          request->send(200, "application/json", data.c_str());
          return;
        }
      }
      request->send(404);
    }
    std::string WebServer::text_sensor_json(text_sensor::TextSensor *obj, const std::string &value,
                                            JsonDetail start_config)
    {
      return json::build_json([this, obj, value, start_config](JsonObject root)
                              {
    set_json_icon_state_value(root, obj, "text_sensor-" + obj->get_object_id(), value, value, start_config);
    if (start_config == DETAIL_ALL) {
      if (this->sorting_entitys_.find(obj) != this->sorting_entitys_.end()) {
        root["sorting_weight"] = this->sorting_entitys_[obj].weight;
        if (this->sorting_groups_.find(this->sorting_entitys_[obj].group_id) != this->sorting_groups_.end()) {
          root["sorting_group"] = this->sorting_groups_[this->sorting_entitys_[obj].group_id].name;
        }
      }
    } });
    }
#endif

#ifdef USE_SWITCH
    void WebServer::on_switch_update(switch_::Switch *obj, bool state)
    {
      if (this->events_.count() == 0)
        return;
      this->events_.send(this->switch_json(obj, state, DETAIL_STATE).c_str(), "state");
    }

    void WebServer::handle_switch_request(AsyncWebServerRequest *request, const UrlMatch &match)
    {
      for (switch_::Switch *obj : App.get_switches())
      {
        if (obj->get_object_id() != match.id)
          continue;

        if (request->method() == HTTP_GET && match.method.empty())
        {
          auto detail = DETAIL_STATE;
          auto *param = request->getParam("detail");
          if (param && param->value() == "all")
          {
            detail = DETAIL_ALL;
          }
          std::string data = this->switch_json(obj, obj->state, detail);
          request->send(200, "application/json", data.c_str());
        }
        else if (match.method == "toggle")
        {
          this->schedule_([obj]()
                          { obj->toggle(); });
          request->send(200);
        }
        else if (match.method == "turn_on")
        {
          this->schedule_([obj]()
                          { obj->turn_on(); });
          request->send(200);
        }
        else if (match.method == "turn_off")
        {
          this->schedule_([obj]()
                          { obj->turn_off(); });
          request->send(200);
        }
        else
        {
          request->send(404);
        }
        return;
      }
      request->send(404);
    }
    std::string WebServer::switch_json(switch_::Switch *obj, bool value, JsonDetail start_config)
    {
      return json::build_json([this, obj, value, start_config](JsonObject root)
                              {
    set_json_icon_state_value(root, obj, "switch-" + obj->get_object_id(), value ? "ON" : "OFF", value, start_config);
    if (start_config == DETAIL_ALL) {
      root["assumed_state"] = obj->assumed_state();
      if (this->sorting_entitys_.find(obj) != this->sorting_entitys_.end()) {
        root["sorting_weight"] = this->sorting_entitys_[obj].weight;
        if (this->sorting_groups_.find(this->sorting_entitys_[obj].group_id) != this->sorting_groups_.end()) {
          root["sorting_group"] = this->sorting_groups_[this->sorting_entitys_[obj].group_id].name;
        }
      }
    } });
    }
#endif

#ifdef USE_BUTTON
    void WebServer::handle_button_request(AsyncWebServerRequest *request, const UrlMatch &match)
    {
      for (button::Button *obj : App.get_buttons())
      {
        if (obj->get_object_id() != match.id)
          continue;
        if (request->method() == HTTP_GET && match.method.empty())
        {
          auto detail = DETAIL_STATE;
          auto *param = request->getParam("detail");
          if (param && param->value() == "all")
          {
            detail = DETAIL_ALL;
          }
          std::string data = this->button_json(obj, detail);
          request->send(200, "application/json", data.c_str());
        }
        else if (match.method == "press")
        {
          this->schedule_([obj]()
                          { obj->press(); });
          request->send(200);
          return;
        }
        else
        {
          request->send(404);
        }
        return;
      }
      request->send(404);
    }
    std::string WebServer::button_json(button::Button *obj, JsonDetail start_config)
    {
      return json::build_json([this, obj, start_config](JsonObject root)
                              {
    set_json_id(root, obj, "button-" + obj->get_object_id(), start_config);
    if (start_config == DETAIL_ALL) {
      if (this->sorting_entitys_.find(obj) != this->sorting_entitys_.end()) {
        root["sorting_weight"] = this->sorting_entitys_[obj].weight;
        if (this->sorting_groups_.find(this->sorting_entitys_[obj].group_id) != this->sorting_groups_.end()) {
          root["sorting_group"] = this->sorting_groups_[this->sorting_entitys_[obj].group_id].name;
        }
      }
    } });
    }
#endif

#ifdef USE_EVENT
    void WebServer::on_event(event::Event *obj, const std::string &event_type)
    {
      this->events_.send(this->event_json(obj, event_type, DETAIL_STATE).c_str(), "state");
    }
    void WebServer::handle_event_request(AsyncWebServerRequest *request, const UrlMatch &match)
    {
      for (event::Event *obj : App.get_events())
      {
        if (obj->get_object_id() != match.id)
          continue;

        if (request->method() == HTTP_GET && match.method.empty())
        {
          auto detail = DETAIL_STATE;
          auto *param = request->getParam("detail");
          if (param && param->value() == "all")
          {
            detail = DETAIL_ALL;
          }
          std::string data = this->event_json(obj, "", detail);
          request->send(200, "application/json", data.c_str());
          return;
        }
      }
      request->send(404);
    }
    std::string WebServer::event_json(event::Event *obj, const std::string &event_type, JsonDetail start_config)
    {
      return json::build_json([this, obj, event_type, start_config](JsonObject root)
                              {
    set_json_id(root, obj, "event-" + obj->get_object_id(), start_config);
    if (!event_type.empty()) {
      root["event_type"] = event_type;
    }
    if (start_config == DETAIL_ALL) {
      JsonArray event_types = root.createNestedArray("event_types");
      for (auto const &event_type : obj->get_event_types()) {
        event_types.add(event_type);
      }
      root["device_class"] = obj->get_device_class();
      if (this->sorting_entitys_.find(obj) != this->sorting_entitys_.end()) {
        root["sorting_weight"] = this->sorting_entitys_[obj].weight;
        if (this->sorting_groups_.find(this->sorting_entitys_[obj].group_id) != this->sorting_groups_.end()) {
          root["sorting_group"] = this->sorting_groups_[this->sorting_entitys_[obj].group_id].name;
        }
      }
    } });
    }
#endif

#ifdef USE_UPDATE
    void WebServer::on_update(update::UpdateEntity *obj)
    {
      if (this->events_.count() == 0)
        return;
      this->events_.send(this->update_json(obj, DETAIL_STATE).c_str(), "state");
    }
    void WebServer::handle_update_request(AsyncWebServerRequest *request, const UrlMatch &match)
    {
      for (update::UpdateEntity *obj : App.get_updates())
      {
        if (obj->get_object_id() != match.id)
          continue;

        if (request->method() == HTTP_GET && match.method.empty())
        {
          auto detail = DETAIL_STATE;
          auto *param = request->getParam("detail");
          if (param && param->value() == "all")
          {
            detail = DETAIL_ALL;
          }
          std::string data = this->update_json(obj, detail);
          request->send(200, "application/json", data.c_str());
          return;
        }

        if (match.method != "install")
        {
          request->send(404);
          return;
        }

        this->schedule_([obj]() mutable
                        { obj->perform(); });
        request->send(200);
        return;
      }
      request->send(404);
    }
    std::string WebServer::update_json(update::UpdateEntity *obj, JsonDetail start_config)
    {
      return json::build_json([this, obj, start_config](JsonObject root)
                              {
    set_json_id(root, obj, "update-" + obj->get_object_id(), start_config);
    root["value"] = obj->update_info.latest_version;
    switch (obj->state) {
      case update::UPDATE_STATE_NO_UPDATE:
        root["state"] = "NO UPDATE";
        break;
      case update::UPDATE_STATE_AVAILABLE:
        root["state"] = "UPDATE AVAILABLE";
        break;
      case update::UPDATE_STATE_INSTALLING:
        root["state"] = "INSTALLING";
        break;
      default:
        root["state"] = "UNKNOWN";
        break;
    }
    if (start_config == DETAIL_ALL) {
      root["current_version"] = obj->update_info.current_version;
      root["title"] = obj->update_info.title;
      root["summary"] = obj->update_info.summary;
      root["release_url"] = obj->update_info.release_url;
      if (this->sorting_entitys_.find(obj) != this->sorting_entitys_.end()) {
        root["sorting_weight"] = this->sorting_entitys_[obj].weight;
        if (this->sorting_groups_.find(this->sorting_entitys_[obj].group_id) != this->sorting_groups_.end()) {
          root["sorting_group"] = this->sorting_groups_[this->sorting_entitys_[obj].group_id].name;
        }
      }
    } });
    }
#endif

    // void WebServer::add_entity_config(EntityBase *entity, float weight, uint64_t group)
    //{
    //     this->sorting_entitys_[entity] = SortingComponents{weight, group};
    // }
    //
    // void WebServer::add_sorting_group(uint64_t group_id, const std::string &group_name, float weight)
    //{
    //    this->sorting_groups_[group_id] = SortingGroup{group_name, weight};
    //}

    void WebServerControllerComponent::schedule_(std::function<void()> &&f)
    {
#ifdef USE_ESP32
      xSemaphoreTake(this->to_schedule_lock_, portMAX_DELAY);
      to_schedule_.push_back(std::move(f));
      xSemaphoreGive(this->to_schedule_lock_);
#else
      this->defer(std::move(f));
#endif
    }

#pragma region AsyncWebHandler_WebServer

    bool AsyncWebHandler_WebServer::canHandle(AsyncWebServerRequest *request)
    {
      ESP_LOGI(TAG, "AsyncWebHandler_WebServer::canHandle %s", request->url().c_str());

      if (request->url() == "/")
        return true;

#ifdef USE_WEBSERVER_CSS_INCLUDE
      if (request->url() == "/0.css")
        return true;
#endif

#ifdef USE_WEBSERVER_JS_INCLUDE
      if (request->url() == "/0.js")
        return true;
#endif

#ifdef USE_WEBSERVER_PRIVATE_NETWORK_ACCESS
      if (request->method() == HTTP_OPTIONS && request->hasHeader(HEADER_CORS_REQ_PNA))
      {
#ifdef USE_ARDUINO
        // Header needs to be added to interesting header list for it to not be
        // nuked by the time we handle the request later.
        // Only required in Arduino framework.
        request->addInterestingHeader(HEADER_CORS_REQ_PNA);
#endif
        return true;
      }
#endif

      UrlMatch match = match_url(request->url().c_str(), true);
      if (!match.valid)
        return false;
#ifdef USE_SENSOR
      if (request->method() == HTTP_GET && match.domain == "sensor")
        return true;
#endif

#ifdef USE_SWITCH
      if ((request->method() == HTTP_POST || request->method() == HTTP_GET) && match.domain == "switch")
        return true;
#endif

#ifdef USE_BUTTON
      if ((request->method() == HTTP_POST || request->method() == HTTP_GET) && match.domain == "button")
        return true;
#endif

#ifdef USE_EVENT
      if (request->method() == HTTP_GET && match.domain == "event")
        return true;
#endif

#ifdef USE_UPDATE
      if ((request->method() == HTTP_POST || request->method() == HTTP_GET) && match.domain == "update")
        return true;
#endif

      return false;
    }

    void AsyncWebHandler_WebServer::handleRequest(AsyncWebServerRequest *request)
    {
      ESP_LOGI(TAG, "AsyncWebHandler_WebServer::handleRequest %s", request->url().c_str());

      if (request->url() == "/")
      {
        this->handle_index_request(request);
        return;
      }

#ifdef USE_WEBSERVER_CSS_INCLUDE
      if (request->url() == "/0.css")
      {
        this->handle_css_request(request);
        return;
      }
#endif

#ifdef USE_WEBSERVER_JS_INCLUDE
      if (request->url() == "/0.js")
      {
        this->handle_js_request(request);
        return;
      }
#endif

#ifdef USE_WEBSERVER_PRIVATE_NETWORK_ACCESS
      if (request->method() == HTTP_OPTIONS && request->hasHeader(HEADER_CORS_REQ_PNA))
      {
        this->handle_pna_cors_request(request);
        return;
      }
#endif

      UrlMatch match = match_url(request->url().c_str());
#ifdef USE_SENSOR
      if (match.domain == "sensor")
      {
        this->handle_sensor_request(request, match);
        return;
      }
#endif

#ifdef USE_SWITCH
      if (match.domain == "switch")
      {
        this->handle_switch_request(request, match);
        return;
      }
#endif

#ifdef USE_BUTTON
      if (match.domain == "button")
      {
        this->handle_button_request(request, match);
        return;
      }
#endif

#ifdef USE_UPDATE
      if (match.domain == "update")
      {
        this->handle_update_request(request, match);
        return;
      }
#endif
    }

    bool AsyncWebHandler_WebServer::isRequestHandlerTrivial()
    {
      return false;
    }

#ifdef USE_WEBSERVER_LOCAL
    void AsyncWebHandler_WebServer::handle_index_request(AsyncWebServerRequest *request)
    {
      AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html", INDEX_GZ, sizeof(INDEX_GZ));
      response->addHeader("Content-Encoding", "gzip");
      request->send(response);
    }
#elif USE_WEBSERVER_VERSION >= 2
    void AsyncWebHandler_WebServer::handle_index_request(AsyncWebServerRequest *request)
    {
      AsyncWebServerResponse *response =
          request->beginResponse_P(200, "text/html", ESPHOME_WEBSERVER_INDEX_HTML, ESPHOME_WEBSERVER_INDEX_HTML_SIZE);
      // No gzip header here because the HTML file is so small
      request->send(response);
    }
#endif

#pragma endregion

  } // namespace web_server
} // namespace esphome
#endif
