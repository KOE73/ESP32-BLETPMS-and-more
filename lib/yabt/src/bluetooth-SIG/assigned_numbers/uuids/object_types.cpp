#include "object_types.hpp"

#include <stddef.h>
#include <sstream>

typedef struct {
    uint16_t value;
    const char* name;
} KeyValue;

static const KeyValue data[] = {
    { 0x2ACA, "Unspecified" },                              // ID: org.bluetooth.object.unspecified
    { 0x2ACB, "Directory Listing" },                        // ID: org.bluetooth.object.directory_listing
    { 0x2BA9, "Media Player Icon" },                        // ID: org.bluetooth.object.media_player_icon
    { 0x2BAA, "Track Segment" },                            // ID: org.bluetooth.object.track_segments
    { 0x2BAB, "Track" },                                    // ID: org.bluetooth.object.track
    { 0x2BAC, "Group" },                                    // ID: org.bluetooth.object.group
};

std::string get_object_types_name(uint16_t code)
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