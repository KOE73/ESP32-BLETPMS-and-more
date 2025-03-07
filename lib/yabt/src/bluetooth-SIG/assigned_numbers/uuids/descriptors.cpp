#include "descriptors.hpp"

#include <stddef.h>
#include <sstream>

typedef struct {
    uint16_t value;
    const char* name;
} KeyValue;

static const KeyValue data[] = {
    { 0x2900, "Characteristic Extended Properties" },       // ID: org.bluetooth.descriptor.gatt.characteristic_extended_properties
    { 0x2901, "Characteristic User Description" },          // ID: org.bluetooth.descriptor.gatt.characteristic_user_description
    { 0x2902, "Client Characteristic Configuration" },      // ID: org.bluetooth.descriptor.gatt.client_characteristic_configuration
    { 0x2903, "Server Characteristic Configuration" },      // ID: org.bluetooth.descriptor.gatt.server_characteristic_configuration
    { 0x2904, "Characteristic Presentation Format" },       // ID: org.bluetooth.descriptor.gatt.characteristic_presentation_format
    { 0x2905, "Characteristic Aggregate Format" },          // ID: org.bluetooth.descriptor.gatt.characteristic_aggregate_format
    { 0x2906, "Valid Range" },                              // ID: org.bluetooth.descriptor.valid_range
    { 0x2907, "External Report Reference" },                // ID: org.bluetooth.descriptor.external_report_reference
    { 0x2908, "Report Reference" },                         // ID: org.bluetooth.descriptor.report_reference
    { 0x2909, "Number of Digitals" },                       // ID: org.bluetooth.descriptor.number_of_digitals
    { 0x290A, "Value Trigger Setting" },                    // ID: org.bluetooth.descriptor.value_trigger_setting
    { 0x290B, "Environmental Sensing Configuration" },      // ID: org.bluetooth.descriptor.es_configuration
    { 0x290C, "Environmental Sensing Measurement" },        // ID: org.bluetooth.descriptor.es_measurement
    { 0x290D, "Environmental Sensing Trigger Setting" },    // ID: org.bluetooth.descriptor.es_trigger_setting
    { 0x290E, "Time Trigger Setting" },                     // ID: org.bluetooth.descriptor.time_trigger_setting
    { 0x290F, "Complete BR-EDR Transport Block Data" },     // ID: org.bluetooth.descriptor.complete_br_edr_transport_block_data
    { 0x2910, "Observation Schedule" },                     // ID: org.bluetooth.descriptor.observation_schedule
    { 0x2911, "Valid Range and Accuracy" },                 // ID: org.bluetooth.descriptor.valid_range_accuracy
    { 0x2912, "Measurement Description" },                  // ID: org.bluetooth.descriptor.measurement_description
    { 0x2913, "Manufacturer Limits" },                      // ID: org.bluetooth.descriptor.manufacturer_limits
    { 0x2914, "Process Tolerances" },                       // ID: org.bluetooth.descriptor.process_tolerances
    { 0x2915, "IMD Trigger Setting" },                      // ID: org.bluetooth.descriptor.imd_trigger_setting
};

std::string get_descriptors_name(uint16_t code)
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