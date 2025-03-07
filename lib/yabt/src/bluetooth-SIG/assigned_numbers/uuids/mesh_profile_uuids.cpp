#include "mesh_profile_uuids.hpp"

#include <stddef.h>
#include <sstream>

typedef struct {
    uint16_t value;
    const char* name;
} KeyValue;

static const KeyValue data[] = {
    { 0x1600, "Ambient Light Sensor NLC Profile 1.0" },
    { 0x1601, "Basic Lightness Controller NLC Profile 1.0" },
    { 0x1602, "Basic Scene Selector NLC Profile 1.0" },
    { 0x1603, "Dimming Control NLC Profile 1.0" },
    { 0x1604, "Energy Monitor NLC Profile 1.0" },
    { 0x1605, "Occupancy Sensor NLC Profile 1.0" },
};

std::string get_mesh_profile_uuids_name(uint16_t code)
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