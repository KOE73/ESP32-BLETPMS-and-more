

#include "string.h"

// #include <iostream>
#include <vector>
#include <memory>
#include <unordered_map>
#include <string>
#include <cstring>
#include <array>
#include <iostream>
#include <iomanip>
#include <map>
#include <optional>
#include <span>
#include <sstream>

// #include "esp_system.h"
// #include "esp_log.h"
// #include "esp_event.h"
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_gap_ble_api.h"

#include "bluetooth-SIG\ad_types.hpp"
//#include "bluetooth-SIG\ad_types.h"


#include "yabt_utils.hpp"



namespace yabt
{

    BtDeviceAddr &BtDeviceAddr::operator=(const esp_bd_addr_t &addr)
    {
        std::copy(std::begin(addr), std::end(addr), this->begin());
        return *this;
    }

    BtDeviceAddr &BtDeviceAddr::operator=(const esp_bd_addr_t *addr)
    {
        std::copy(std::begin(*addr), std::end(*addr), this->begin());
        return *this;
    }

    BtDeviceAddr::BtDeviceAddr(const BtDeviceAddrSpan &span)
    {
        std::copy(span.data.begin(), span.data.end(), this->begin());
    }

    BtDeviceAddr &BtDeviceAddr::operator=(const BtDeviceAddrSpan &span)
    {
        std::copy(span.data.begin(), span.data.end(), this->begin());
        return *this;
    }

    bool BtDeviceAddr::operator==(const esp_bd_addr_t &addr) const
    {
        return std::equal(this->begin(), this->end(), std::begin(addr));
    }

    bool BtDeviceAddr::operator!=(const esp_bd_addr_t &addr) const
    {
        return !(*this == addr);
    }

    bool BtDeviceAddr::operator==(const BtDeviceAddrSpan &span) const
    {
        return std::equal(this->begin(), this->end(), span.data.begin());
    }

    bool BtDeviceAddr::operator!=(const BtDeviceAddrSpan &span) const
    {
        return !(*this == span);
    }

    std::string BtDeviceAddr::toString() const
    {
        std::ostringstream oss;
        oss << std::hex << std::uppercase << std::setfill('0'); // Настраиваем формат: HEX, заглавные буквы, заполнение нулями

        // Используем итераторы для обхода массива
        for (auto it = begin(); it != end(); ++it)
        {
            oss << std::setw(2) << static_cast<unsigned>(*it);
            if (it != end() - 1)
                oss << ":";
        }

        return oss.str();
    }

    bool BtDeviceAddrSpan::operator==(const esp_bd_addr_t &addr) const
    {
        return std::equal(data.begin(), data.end(), std::begin(addr));
    }

    bool BtDeviceAddrSpan::operator!=(const esp_bd_addr_t &addr) const
    {
        return !(*this == addr);
    }

    bool BtDeviceAddrSpan::operator==(const BtDeviceAddr &addr) const
    {
        return std::equal(data.begin(), data.end(), addr.begin());
    }

    bool BtDeviceAddrSpan::operator!=(const BtDeviceAddr &addr) const
    {
        return !(*this == addr);
    }

    std::string BtDeviceAddrSpan::toString() const
    {
        std::ostringstream oss;
        oss << std::hex << std::uppercase << std::setfill('0'); // HEX, заглавные, нули

        for (auto it = data.begin(); it != data.end(); ++it)
        {
            oss << std::setw(2) << static_cast<unsigned>(*it);
            if (it != data.end() - 1)
                oss << ":";
        }

        return oss.str();
    }

} // namespace yabt
