#include "ad_types.hpp"

#include <stddef.h>
#include <sstream>

typedef struct {
    uint8_t value;
    const char* name;
} KeyValue;

static const KeyValue data[] = {
    { 0x0001, "Flags" },
    { 0x0002, "Incomplete List of 16-bit Service or Service Class UUIDs" },
    { 0x0003, "Complete List of 16-bit Service or Service Class UUIDs" },
    { 0x0004, "Incomplete List of 32-bit Service or Service Class UUIDs" },
    { 0x0005, "Complete List of 32-bit Service or Service Class UUIDs" },
    { 0x0006, "Incomplete List of 128-bit Service or Service Class UUIDs" },
    { 0x0007, "Complete List of 128-bit Service or Service Class UUIDs" },
    { 0x0008, "Shortened Local Name" },
    { 0x0009, "Complete Local Name" },
    { 0x000A, "Tx Power Level" },
    { 0x000D, "Class of Device" },
    { 0x000E, "Simple Pairing Hash C-192" },
    { 0x000F, "Simple Pairing Randomizer R-192" },
    { 0x0010, "Device ID" },
    { 0x0010, "Security Manager TK Value" },
    { 0x0011, "Security Manager Out of Band Flags" },
    { 0x0012, "Peripheral Connection Interval Range" },
    { 0x0014, "List of 16-bit Service Solicitation UUIDs" },
    { 0x0015, "List of 128-bit Service Solicitation UUIDs" },
    { 0x0016, "Service Data - 16-bit UUID" },
    { 0x0017, "Public Target Address" },
    { 0x0018, "Random Target Address" },
    { 0x0019, "Appearance" },
    { 0x001A, "Advertising Interval" },
    { 0x001B, "LE Bluetooth Device Address" },
    { 0x001C, "LE Role" },
    { 0x001D, "Simple Pairing Hash C-256" },
    { 0x001E, "Simple Pairing Randomizer R-256" },
    { 0x001F, "List of 32-bit Service Solicitation UUIDs" },
    { 0x0020, "Service Data - 32-bit UUID" },
    { 0x0021, "Service Data - 128-bit UUID" },
    { 0x0022, "LE Secure Connections Confirmation Value" },
    { 0x0023, "LE Secure Connections Random Value" },
    { 0x0024, "URI" },
    { 0x0025, "Indoor Positioning" },
    { 0x0026, "Transport Discovery Data" },
    { 0x0027, "LE Supported Features" },
    { 0x0028, "Channel Map Update Indication" },
    { 0x0029, "PB-ADV" },
    { 0x002A, "Mesh Message" },
    { 0x002B, "Mesh Beacon" },
    { 0x002C, "BIGInfo" },
    { 0x002D, "Broadcast_Code" },
    { 0x002E, "Resolvable Set Identifier" },
    { 0x002F, "Advertising Interval - long" },
    { 0x0030, "Broadcast_Name" },
    { 0x0031, "Encrypted Advertising Data" },
    { 0x0032, "Periodic Advertising Response Timing Information" },
    { 0x0034, "Electronic Shelf Label" },
    { 0x003D, "3D Information Data" },
    { 0x00FF, "Manufacturer Specific Data" },
};

std::string get_ad_types_name(uint8_t code)
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