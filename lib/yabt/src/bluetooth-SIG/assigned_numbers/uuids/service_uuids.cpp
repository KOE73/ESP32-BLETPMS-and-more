#include "service_uuids.hpp"

#include <stddef.h>
#include <sstream>

typedef struct {
    uint16_t value;
    const char* name;
} KeyValue;

static const KeyValue data[] = {
    { 0x1800, "GAP" },                                      // ID: org.bluetooth.service.gap
    { 0x1801, "GATT" },                                     // ID: org.bluetooth.service.gatt
    { 0x1802, "Immediate Alert" },                          // ID: org.bluetooth.service.immediate_alert
    { 0x1803, "Link Loss" },                                // ID: org.bluetooth.service.link_loss
    { 0x1804, "Tx Power" },                                 // ID: org.bluetooth.service.tx_power
    { 0x1805, "Current Time" },                             // ID: org.bluetooth.service.current_time
    { 0x1806, "Reference Time Update" },                    // ID: org.bluetooth.service.reference_time_update
    { 0x1807, "Next DST Change" },                          // ID: org.bluetooth.service.next_dst_change
    { 0x1808, "Glucose" },                                  // ID: org.bluetooth.service.glucose
    { 0x1809, "Health Thermometer" },                       // ID: org.bluetooth.service.health_thermometer
    { 0x180A, "Device Information" },                       // ID: org.bluetooth.service.device_information
    { 0x180D, "Heart Rate" },                               // ID: org.bluetooth.service.heart_rate
    { 0x180E, "Phone Alert Status" },                       // ID: org.bluetooth.service.phone_alert_status
    { 0x180F, "Battery" },                                  // ID: org.bluetooth.service.battery_service
    { 0x1810, "Blood Pressure" },                           // ID: org.bluetooth.service.blood_pressure
    { 0x1811, "Alert Notification" },                       // ID: org.bluetooth.service.alert_notification
    { 0x1812, "Human Interface Device" },                   // ID: org.bluetooth.service.human_interface_device
    { 0x1813, "Scan Parameters" },                          // ID: org.bluetooth.service.scan_parameters
    { 0x1814, "Running Speed and Cadence" },                // ID: org.bluetooth.service.running_speed_and_cadence
    { 0x1815, "Automation IO" },                            // ID: org.bluetooth.service.automation_io
    { 0x1816, "Cycling Speed and Cadence" },                // ID: org.bluetooth.service.cycling_speed_and_cadence
    { 0x1818, "Cycling Power" },                            // ID: org.bluetooth.service.cycling_power
    { 0x1819, "Location and Navigation" },                  // ID: org.bluetooth.service.location_and_navigation
    { 0x181A, "Environmental Sensing" },                    // ID: org.bluetooth.service.environmental_sensing
    { 0x181B, "Body Composition" },                         // ID: org.bluetooth.service.body_composition
    { 0x181C, "User Data" },                                // ID: org.bluetooth.service.user_data
    { 0x181D, "Weight Scale" },                             // ID: org.bluetooth.service.weight_scale
    { 0x181E, "Bond Management" },                          // ID: org.bluetooth.service.bond_management
    { 0x181F, "Continuous Glucose Monitoring" },            // ID: org.bluetooth.service.continuous_glucose_monitoring
    { 0x1820, "Internet Protocol Support" },                // ID: org.bluetooth.service.internet_protocol_support
    { 0x1821, "Indoor Positioning" },                       // ID: org.bluetooth.service.indoor_positioning
    { 0x1822, "Pulse Oximeter" },                           // ID: org.bluetooth.service.pulse_oximeter
    { 0x1823, "HTTP Proxy" },                               // ID: org.bluetooth.service.http_proxy
    { 0x1824, "Transport Discovery" },                      // ID: org.bluetooth.service.transport_discovery
    { 0x1825, "Object Transfer" },                          // ID: org.bluetooth.service.object_transfer
    { 0x1826, "Fitness Machine" },                          // ID: org.bluetooth.service.fitness_machine
    { 0x1827, "Mesh Provisioning" },                        // ID: org.bluetooth.service.mesh_provisioning
    { 0x1828, "Mesh Proxy" },                               // ID: org.bluetooth.service.mesh_proxy
    { 0x1829, "Reconnection Configuration" },               // ID: org.bluetooth.service.reconnection_configuration
    { 0x183A, "Insulin Delivery" },                         // ID: org.bluetooth.service.insulin_delivery
    { 0x183B, "Binary Sensor" },                            // ID: org.bluetooth.service.binary_sensor
    { 0x183C, "Emergency Configuration" },                  // ID: org.bluetooth.service.emergency_configuration
    { 0x183D, "Authorization Control" },                    // ID: org.bluetooth.service.authorization_control
    { 0x183E, "Physical Activity Monitor" },                // ID: org.bluetooth.service.physical_activity_monitor
    { 0x183F, "Elapsed Time" },                             // ID: org.bluetooth.service.elapsed_time
    { 0x1840, "Generic Health Sensor" },                    // ID: org.bluetooth.service.generic_health_sensor
    { 0x1843, "Audio Input Control" },                      // ID: org.bluetooth.service.audio_input_control
    { 0x1844, "Volume Control" },                           // ID: org.bluetooth.service.volume_control
    { 0x1845, "Volume Offset Control" },                    // ID: org.bluetooth.service.volume_offset
    { 0x1846, "Coordinated Set Identification" },           // ID: org.bluetooth.service.coordinated_set_identification
    { 0x1847, "Device Time" },                              // ID: org.bluetooth.service.device_time
    { 0x1848, "Media Control" },                            // ID: org.bluetooth.service.media_control
    { 0x1849, "Generic Media Control" },                    // ID: org.bluetooth.service.generic_media_control
    { 0x184A, "Constant Tone Extension" },                  // ID: org.bluetooth.service.constant_tone_extension
    { 0x184B, "Telephone Bearer" },                         // ID: org.bluetooth.service.telephone_bearer
    { 0x184C, "Generic Telephone Bearer" },                 // ID: org.bluetooth.service.generic_telephone_bearer
    { 0x184D, "Microphone Control" },                       // ID: org.bluetooth.service.microphone_control
    { 0x184E, "Audio Stream Control" },                     // ID: org.bluetooth.service.audio_stream_control
    { 0x184F, "Broadcast Audio Scan" },                     // ID: org.bluetooth.service.broadcast_audio_scan
    { 0x1850, "Published Audio Capabilities" },             // ID: org.bluetooth.service.published_audio_capabilities
    { 0x1851, "Basic Audio Announcement" },                 // ID: org.bluetooth.service.basic_audio_announcement
    { 0x1852, "Broadcast Audio Announcement" },             // ID: org.bluetooth.service.broadcast_audio_announcement
    { 0x1853, "Common Audio" },                             // ID: org.bluetooth.service.common_audio
    { 0x1854, "Hearing Access" },                           // ID: org.bluetooth.service.hearing_access
    { 0x1855, "Telephony and Media Audio" },                // ID: org.bluetooth.service.telephony_and_media_audio
    { 0x1856, "Public Broadcast Announcement" },            // ID: org.bluetooth.service.public_broadcast_announcement
    { 0x1857, "Electronic Shelf Label" },                   // ID: org.bluetooth.service.electronic_shelf_label
    { 0x1858, "Gaming Audio" },                             // ID: org.bluetooth.service.gaming_audio
    { 0x1859, "Mesh Proxy Solicitation" },                  // ID: org.bluetooth.service.mesh_proxy_solicitation
    { 0x185A, "Industrial Measurement Device" },            // ID: org.bluetooth.service.industrial_measurement_device
    { 0x185B, "Ranging" },                                  // ID: org.bluetooth.service.ranging
};

std::string get_service_uuids_name(uint16_t code)
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