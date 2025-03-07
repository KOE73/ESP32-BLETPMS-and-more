#include "sdo_uuids.hpp"

#include <stddef.h>
#include <sstream>

typedef struct {
    uint16_t value;
    const char* name;
} KeyValue;

static const KeyValue data[] = {
    { 0xFCCC, "Wi-Fi Easy Connect Specification" },
    { 0xFFEF, "Wi-Fi Direct Specification" },
    { 0xFFF0, "Public Key Open Credential (PKOC)" },
    { 0xFFF1, "ICCE Digital Key" },
    { 0xFFF2, "Aliro" },
    { 0xFFF3, "FiRa Consortium" },
    { 0xFFF4, "FiRa Consortium" },
    { 0xFFF5, "Car Connectivity Consortium, LLC" },
    { 0xFFF6, "Matter Profile ID" },
    { 0xFFF7, "Zigbee Direct" },
    { 0xFFF8, "Mopria Alliance BLE" },
    { 0xFFF9, "FIDO2 secure client-to-authenticator transport" },
    { 0xFFFA, "ASTM Remote ID" },
    { 0xFFFB, "Direct Thread Commissioning" },
    { 0xFFFC, "Wireless Power Transfer (WPT)" },
    { 0xFFFD, "Universal Second Factor Authenticator" },
    { 0xFFFE, "Wireless Power Transfer" },
};

std::string get_sdo_uuids_name(uint16_t code)
{
    for (size_t i = 0; data[i].name != nullptr; i++)
    {
        if (data[i].value == code)
        {
            return data[i].name;
        }
    }
    std::ostringstream oss;
    oss << "Unknown [" << static_cast<int>(code) << "]";
    return oss.str();
}