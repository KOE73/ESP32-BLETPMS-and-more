
#ifdef USE_WEBSERVER
#include "web_server.h"
#include "web_url_match.hpp"

#include "json_util.h"
#include "helpers.h"
#include "esp_log.h"

#include <cstdlib>

namespace esphome
{
  namespace web_server
  {

    /// @brief Разбор проинятого url. В перспективе переделать в класс?
    /// @param url
    /// @param only_domain
    /// @return Структура с данными, что там в строке
    UrlMatch match_url(const std::string &url, bool only_domain /* = false*/)
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

  } // namespace web_server
} // namespace esphome
#endif
