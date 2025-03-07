#include "declarations.hpp"

#include <stddef.h>
#include <sstream>

typedef struct {
    uint16_t value;
    const char* name;
} KeyValue;

static const KeyValue data[] = {
    { 0x2800, "Primary Service" },                          // ID: org.bluetooth.attribute.gatt.primary_service_declaration
    { 0x2801, "Secondary Service" },                        // ID: org.bluetooth.attribute.gatt.secondary_service_declaration
    { 0x2802, "Include" },                                  // ID: org.bluetooth.attribute.gatt.include_declaration
    { 0x2803, "Characteristic" },                           // ID: org.bluetooth.attribute.gatt.characteristic_declaration
};

std::string get_declarations_name(uint16_t code)
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