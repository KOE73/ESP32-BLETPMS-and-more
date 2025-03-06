#include "ad_types.hpp"

#include <stddef.h>
#include <sstream>


typedef struct
{
    uint8_t value;
    const char *name;
} KeyValue;

static const KeyValue data[] = {
    {0x01, "Flags"},
    {0x02, "Incomplete List of 16-bit Service or Service Class UUIDs"},
    {0x03, "Complete List of 16-bit Service or Service Class UUIDs"},
    {0x04, "Incomplete List of 32-bit Service or Service Class UUIDs"},
    {0x05, "Complete List of 32-bit Service or Service Class UUIDs"},
    {0x06, "Incomplete List of 128-bit Service or Service Class UUIDs"},
    {0x07, "Complete List of 128-bit Service or Service Class UUIDs"},
    {0x08, "Shortened Local Name"},
    {0x09, "Complete Local Name"},
    {0x0A, "Tx Power Level"},
    {0x0D, "Class of Device"},
    {0x0E, "Simple Pairing Hash C-192"},
    {0x0F, "Simple Pairing Randomizer R-192"},
    {0x10, "Device ID"},
    {0x10, "Security Manager TK Value"},
    {0x11, "Security Manager Out of Band Flags"},
    {0x12, "Peripheral Connection Interval Range"},
    {0x14, "List of 16-bit Service Solicitation UUIDs"},
    {0x15, "List of 128-bit Service Solicitation UUIDs"},
    {0x16, "Service Data - 16-bit UUID"},
    {0x17, "Public Target Address"},
    {0x18, "Random Target Address"},
    {0x19, "Appearance"},
    {0x1A, "Advertising Interval"},
    {0x1B, "LE Bluetooth Device Address"},
    {0x1C, "LE Role"},
    {0x1D, "Simple Pairing Hash C-256"},
    {0x1E, "Simple Pairing Randomizer R-256"},
    {0x1F, "List of 32-bit Service Solicitation UUIDs"},
    {0x20, "Service Data - 32-bit UUID"},
    {0x21, "Service Data - 128-bit UUID"},
    {0x22, "LE Secure Connections Confirmation Value"},
    {0x23, "LE Secure Connections Random Value"},
    {0x24, "URI"},
    {0x25, "Indoor Positioning"},
    {0x26, "Transport Discovery Data"},
    {0x27, "LE Supported Features"},
    {0x28, "Channel Map Update Indication"},
    {0x29, "PB-ADV"},
    {0x2A, "Mesh Message"},
    {0x2B, "Mesh Beacon"},
    {0x2C, "BIGInfo"},
    {0x2D, "Broadcast_Code"},
    {0x2E, "Resolvable Set Identifier"},
    {0x2F, "Advertising Interval - long"},
    {0x30, "Broadcast_Name"},
    {0x31, "Encrypted Advertising Data"},
    {0x32, "Periodic Advertising Response Timing Information"},
    {0x34, "Electronic Shelf Label"},
    {0x3D, "3D Information Data"},
    {0xFF, "Manufacturer Specific Data"},
    {0, NULL}};

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
