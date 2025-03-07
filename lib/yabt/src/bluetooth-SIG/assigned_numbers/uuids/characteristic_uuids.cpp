#include "characteristic_uuids.hpp"

#include <stddef.h>
#include <sstream>

typedef struct {
    uint16_t value;
    const char* name;
} KeyValue;

static const KeyValue data[] = {
    { 0x2A00, "Device Name" },                              // ID: org.bluetooth.characteristic.gap.device_name
    { 0x2A01, "Appearance" },                               // ID: org.bluetooth.characteristic.gap.appearance
    { 0x2A02, "Peripheral Privacy Flag" },                  // ID: org.bluetooth.characteristic.gap.peripheral_privacy_flag
    { 0x2A03, "Reconnection Address" },                     // ID: org.bluetooth.characteristic.gap.reconnection_address
    { 0x2A04, "Peripheral Preferred Connection Parameters" },// ID: org.bluetooth.characteristic.gap.peripheral_preferred_connection_parameters
    { 0x2A05, "Service Changed" },                          // ID: org.bluetooth.characteristic.gatt.service_changed
    { 0x2A06, "Alert Level" },                              // ID: org.bluetooth.characteristic.alert_level
    { 0x2A07, "Tx Power Level" },                           // ID: org.bluetooth.characteristic.tx_power_level
    { 0x2A08, "Date Time" },                                // ID: org.bluetooth.characteristic.date_time
    { 0x2A09, "Day of Week" },                              // ID: org.bluetooth.characteristic.day_of_week
    { 0x2A0A, "Day Date Time" },                            // ID: org.bluetooth.characteristic.day_date_time
    { 0x2A0C, "Exact Time 256" },                           // ID: org.bluetooth.characteristic.exact_time_256
    { 0x2A0D, "DST Offset" },                               // ID: org.bluetooth.characteristic.dst_offset
    { 0x2A0E, "Time Zone" },                                // ID: org.bluetooth.characteristic.time_zone
    { 0x2A0F, "Local Time Information" },                   // ID: org.bluetooth.characteristic.local_time_information
    { 0x2A11, "Time with DST" },                            // ID: org.bluetooth.characteristic.time_with_dst
    { 0x2A12, "Time Accuracy" },                            // ID: org.bluetooth.characteristic.time_accuracy
    { 0x2A13, "Time Source" },                              // ID: org.bluetooth.characteristic.time_source
    { 0x2A14, "Reference Time Information" },               // ID: org.bluetooth.characteristic.reference_time_information
    { 0x2A16, "Time Update Control Point" },                // ID: org.bluetooth.characteristic.time_update_control_point
    { 0x2A17, "Time Update State" },                        // ID: org.bluetooth.characteristic.time_update_state
    { 0x2A18, "Glucose Measurement" },                      // ID: org.bluetooth.characteristic.glucose_measurement
    { 0x2A19, "Battery Level" },                            // ID: org.bluetooth.characteristic.battery_level
    { 0x2A1C, "Temperature Measurement" },                  // ID: org.bluetooth.characteristic.temperature_measurement
    { 0x2A1D, "Temperature Type" },                         // ID: org.bluetooth.characteristic.temperature_type
    { 0x2A1E, "Intermediate Temperature" },                 // ID: org.bluetooth.characteristic.intermediate_temperature
    { 0x2A21, "Measurement Interval" },                     // ID: org.bluetooth.characteristic.measurement_interval
    { 0x2A22, "Boot Keyboard Input Report" },               // ID: org.bluetooth.characteristic.boot_keyboard_input_report
    { 0x2A23, "System ID" },                                // ID: org.bluetooth.characteristic.system_id
    { 0x2A24, "Model Number String" },                      // ID: org.bluetooth.characteristic.model_number_string
    { 0x2A25, "Serial Number String" },                     // ID: org.bluetooth.characteristic.serial_number_string
    { 0x2A26, "Firmware Revision String" },                 // ID: org.bluetooth.characteristic.firmware_revision_string
    { 0x2A27, "Hardware Revision String" },                 // ID: org.bluetooth.characteristic.hardware_revision_string
    { 0x2A28, "Software Revision String" },                 // ID: org.bluetooth.characteristic.software_revision_string
    { 0x2A29, "Manufacturer Name String" },                 // ID: org.bluetooth.characteristic.manufacturer_name_string
    { 0x2A2A, "IEEE 11073-20601 Regulatory Certification Data List" },// ID: org.bluetooth.characteristic.ieee_11073-20601_regulatory_certification_data_list
    { 0x2A2B, "Current Time" },                             // ID: org.bluetooth.characteristic.current_time
    { 0x2A2C, "Magnetic Declination" },                     // ID: org.bluetooth.characteristic.magnetic_declination
    { 0x2A31, "Scan Refresh" },                             // ID: org.bluetooth.characteristic.scan_refresh
    { 0x2A32, "Boot Keyboard Output Report" },              // ID: org.bluetooth.characteristic.boot_keyboard_output_report
    { 0x2A33, "Boot Mouse Input Report" },                  // ID: org.bluetooth.characteristic.boot_mouse_input_report
    { 0x2A34, "Glucose Measurement Context" },              // ID: org.bluetooth.characteristic.glucose_measurement_context
    { 0x2A35, "Blood Pressure Measurement" },               // ID: org.bluetooth.characteristic.blood_pressure_measurement
    { 0x2A36, "Intermediate Cuff Pressure" },               // ID: org.bluetooth.characteristic.intermediate_cuff_pressure
    { 0x2A37, "Heart Rate Measurement" },                   // ID: org.bluetooth.characteristic.heart_rate_measurement
    { 0x2A38, "Body Sensor Location" },                     // ID: org.bluetooth.characteristic.body_sensor_location
    { 0x2A39, "Heart Rate Control Point" },                 // ID: org.bluetooth.characteristic.heart_rate_control_point
    { 0x2A3F, "Alert Status" },                             // ID: org.bluetooth.characteristic.alert_status
    { 0x2A40, "Ringer Control Point" },                     // ID: org.bluetooth.characteristic.ringer_control_point
    { 0x2A41, "Ringer Setting" },                           // ID: org.bluetooth.characteristic.ringer_setting
    { 0x2A42, "Alert Category ID Bit Mask" },               // ID: org.bluetooth.characteristic.alert_category_id_bit_mask
    { 0x2A43, "Alert Category ID" },                        // ID: org.bluetooth.characteristic.alert_category_id
    { 0x2A44, "Alert Notification Control Point" },         // ID: org.bluetooth.characteristic.alert_notification_control_point
    { 0x2A45, "Unread Alert Status" },                      // ID: org.bluetooth.characteristic.unread_alert_status
    { 0x2A46, "New Alert" },                                // ID: org.bluetooth.characteristic.new_alert
    { 0x2A47, "Supported New Alert Category" },             // ID: org.bluetooth.characteristic.supported_new_alert_category
    { 0x2A48, "Supported Unread Alert Category" },          // ID: org.bluetooth.characteristic.supported_unread_alert_category
    { 0x2A49, "Blood Pressure Feature" },                   // ID: org.bluetooth.characteristic.blood_pressure_feature
    { 0x2A4A, "HID Information" },                          // ID: org.bluetooth.characteristic.hid_information
    { 0x2A4B, "Report Map" },                               // ID: org.bluetooth.characteristic.report_map
    { 0x2A4C, "HID Control Point" },                        // ID: org.bluetooth.characteristic.hid_control_point
    { 0x2A4D, "Report" },                                   // ID: org.bluetooth.characteristic.report
    { 0x2A4E, "Protocol Mode" },                            // ID: org.bluetooth.characteristic.protocol_mode
    { 0x2A4F, "Scan Interval Window" },                     // ID: org.bluetooth.characteristic.scan_interval_window
    { 0x2A50, "PnP ID" },                                   // ID: org.bluetooth.characteristic.pnp_id
    { 0x2A51, "Glucose Feature" },                          // ID: org.bluetooth.characteristic.glucose_feature
    { 0x2A52, "Record Access Control Point" },              // ID: org.bluetooth.characteristic.record_access_control_point
    { 0x2A53, "RSC Measurement" },                          // ID: org.bluetooth.characteristic.rsc_measurement
    { 0x2A54, "RSC Feature" },                              // ID: org.bluetooth.characteristic.rsc_feature
    { 0x2A55, "SC Control Point" },                         // ID: org.bluetooth.characteristic.sc_control_point
    { 0x2A5A, "Aggregate" },                                // ID: org.bluetooth.characteristic.aggregate
    { 0x2A5B, "CSC Measurement" },                          // ID: org.bluetooth.characteristic.csc_measurement
    { 0x2A5C, "CSC Feature" },                              // ID: org.bluetooth.characteristic.csc_feature
    { 0x2A5D, "Sensor Location" },                          // ID: org.bluetooth.characteristic.sensor_location
    { 0x2A5E, "PLX Spot-Check Measurement" },               // ID: org.bluetooth.characteristic.plx_spot_check_measurement
    { 0x2A5F, "PLX Continuous Measurement" },               // ID: org.bluetooth.characteristic.plx_continuous_measurement
    { 0x2A60, "PLX Features" },                             // ID: org.bluetooth.characteristic.plx_features
    { 0x2A63, "Cycling Power Measurement" },                // ID: org.bluetooth.characteristic.cycling_power_measurement
    { 0x2A64, "Cycling Power Vector" },                     // ID: org.bluetooth.characteristic.cycling_power_vector
    { 0x2A65, "Cycling Power Feature" },                    // ID: org.bluetooth.characteristic.cycling_power_feature
    { 0x2A66, "Cycling Power Control Point" },              // ID: org.bluetooth.characteristic.cycling_power_control_point
    { 0x2A67, "Location and Speed" },                       // ID: org.bluetooth.characteristic.location_and_speed
    { 0x2A68, "Navigation" },                               // ID: org.bluetooth.characteristic.navigation
    { 0x2A69, "Position Quality" },                         // ID: org.bluetooth.characteristic.position_quality
    { 0x2A6A, "LN Feature" },                               // ID: org.bluetooth.characteristic.ln_feature
    { 0x2A6B, "LN Control Point" },                         // ID: org.bluetooth.characteristic.ln_control_point
    { 0x2A6C, "Elevation" },                                // ID: org.bluetooth.characteristic.elevation
    { 0x2A6D, "Pressure" },                                 // ID: org.bluetooth.characteristic.pressure
    { 0x2A6E, "Temperature" },                              // ID: org.bluetooth.characteristic.temperature
    { 0x2A6F, "Humidity" },                                 // ID: org.bluetooth.characteristic.humidity
    { 0x2A70, "True Wind Speed" },                          // ID: org.bluetooth.characteristic.true_wind_speed
    { 0x2A71, "True Wind Direction" },                      // ID: org.bluetooth.characteristic.true_wind_direction
    { 0x2A72, "Apparent Wind Speed" },                      // ID: org.bluetooth.characteristic.apparent_wind_speed
    { 0x2A73, "Apparent Wind Direction" },                  // ID: org.bluetooth.characteristic.apparent_wind_direction
    { 0x2A74, "Gust Factor" },                              // ID: org.bluetooth.characteristic.gust_factor
    { 0x2A75, "Pollen Concentration" },                     // ID: org.bluetooth.characteristic.pollen_concentration
    { 0x2A76, "UV Index" },                                 // ID: org.bluetooth.characteristic.uv_index
    { 0x2A77, "Irradiance" },                               // ID: org.bluetooth.characteristic.irradiance
    { 0x2A78, "Rainfall" },                                 // ID: org.bluetooth.characteristic.rainfall
    { 0x2A79, "Wind Chill" },                               // ID: org.bluetooth.characteristic.wind_chill
    { 0x2A7A, "Heat Index" },                               // ID: org.bluetooth.characteristic.heat_index
    { 0x2A7B, "Dew Point" },                                // ID: org.bluetooth.characteristic.dew_point
    { 0x2A7D, "Descriptor Value Changed" },                 // ID: org.bluetooth.characteristic.descriptor_value_changed
    { 0x2A7E, "Aerobic Heart Rate Lower Limit" },           // ID: org.bluetooth.characteristic.aerobic_heart_rate_lower_limit
    { 0x2A7F, "Aerobic Threshold" },                        // ID: org.bluetooth.characteristic.aerobic_threshold
    { 0x2A80, "Age" },                                      // ID: org.bluetooth.characteristic.age
    { 0x2A81, "Anaerobic Heart Rate Lower Limit" },         // ID: org.bluetooth.characteristic.anaerobic_heart_rate_lower_limit
    { 0x2A82, "Anaerobic Heart Rate Upper Limit" },         // ID: org.bluetooth.characteristic.anaerobic_heart_rate_upper_limit
    { 0x2A83, "Anaerobic Threshold" },                      // ID: org.bluetooth.characteristic.anaerobic_threshold
    { 0x2A84, "Aerobic Heart Rate Upper Limit" },           // ID: org.bluetooth.characteristic.aerobic_heart_rate_upper_limit
    { 0x2A85, "Date of Birth" },                            // ID: org.bluetooth.characteristic.date_of_birth
    { 0x2A86, "Date of Threshold Assessment" },             // ID: org.bluetooth.characteristic.date_of_threshold_assessment
    { 0x2A87, "Email Address" },                            // ID: org.bluetooth.characteristic.email_address
    { 0x2A88, "Fat Burn Heart Rate Lower Limit" },          // ID: org.bluetooth.characteristic.fat_burn_heart_rate_lower_limit
    { 0x2A89, "Fat Burn Heart Rate Upper Limit" },          // ID: org.bluetooth.characteristic.fat_burn_heart_rate_upper_limit
    { 0x2A8A, "First Name" },                               // ID: org.bluetooth.characteristic.first_name
    { 0x2A8B, "Five Zone Heart Rate Limits" },              // ID: org.bluetooth.characteristic.five_zone_heart_rate_limits
    { 0x2A8C, "Gender" },                                   // ID: org.bluetooth.characteristic.gender
    { 0x2A8D, "Heart Rate Max" },                           // ID: org.bluetooth.characteristic.heart_rate_max
    { 0x2A8E, "Height" },                                   // ID: org.bluetooth.characteristic.height
    { 0x2A8F, "Hip Circumference" },                        // ID: org.bluetooth.characteristic.hip_circumference
    { 0x2A90, "Last Name" },                                // ID: org.bluetooth.characteristic.last_name
    { 0x2A91, "Maximum Recommended Heart Rate" },           // ID: org.bluetooth.characteristic.maximum_recommended_heart_rate
    { 0x2A92, "Resting Heart Rate" },                       // ID: org.bluetooth.characteristic.resting_heart_rate
    { 0x2A93, "Sport Type for Aerobic and Anaerobic Thresholds" },// ID: org.bluetooth.characteristic.sport_type_for_aerobic_and_anaerobic_thresholds
    { 0x2A94, "Three Zone Heart Rate Limits" },             // ID: org.bluetooth.characteristic.three_zone_heart_rate_limits
    { 0x2A95, "Two Zone Heart Rate Limits" },               // ID: org.bluetooth.characteristic.two_zone_heart_rate_limits
    { 0x2A96, "VO2 Max" },                                  // ID: org.bluetooth.characteristic.vo2_max
    { 0x2A97, "Waist Circumference" },                      // ID: org.bluetooth.characteristic.waist_circumference
    { 0x2A98, "Weight" },                                   // ID: org.bluetooth.characteristic.weight
    { 0x2A99, "Database Change Increment" },                // ID: org.bluetooth.characteristic.database_change_increment
    { 0x2A9A, "User Index" },                               // ID: org.bluetooth.characteristic.user_index
    { 0x2A9B, "Body Composition Feature" },                 // ID: org.bluetooth.characteristic.body_composition_feature
    { 0x2A9C, "Body Composition Measurement" },             // ID: org.bluetooth.characteristic.body_composition_measurement
    { 0x2A9D, "Weight Measurement" },                       // ID: org.bluetooth.characteristic.weight_measurement
    { 0x2A9E, "Weight Scale Feature" },                     // ID: org.bluetooth.characteristic.weight_scale_feature
    { 0x2A9F, "User Control Point" },                       // ID: org.bluetooth.characteristic.user_control_point
    { 0x2AA0, "Magnetic Flux Density - 2D" },               // ID: org.bluetooth.characteristic.magnetic_flux_density_2d
    { 0x2AA1, "Magnetic Flux Density - 3D" },               // ID: org.bluetooth.characteristic.magnetic_flux_density_3d
    { 0x2AA2, "Language" },                                 // ID: org.bluetooth.characteristic.language
    { 0x2AA3, "Barometric Pressure Trend" },                // ID: org.bluetooth.characteristic.barometric_pressure_trend
    { 0x2AA4, "Bond Management Control Point" },            // ID: org.bluetooth.characteristic.bond_management_control_point
    { 0x2AA5, "Bond Management Feature" },                  // ID: org.bluetooth.characteristic.bond_management_feature
    { 0x2AA6, "Central Address Resolution" },               // ID: org.bluetooth.characteristic.gap.central_address_resolution
    { 0x2AA7, "CGM Measurement" },                          // ID: org.bluetooth.characteristic.cgm_measurement
    { 0x2AA8, "CGM Feature" },                              // ID: org.bluetooth.characteristic.cgm_feature
    { 0x2AA9, "CGM Status" },                               // ID: org.bluetooth.characteristic.cgm_status
    { 0x2AAA, "CGM Session Start Time" },                   // ID: org.bluetooth.characteristic.cgm_session_start_time
    { 0x2AAB, "CGM Session Run Time" },                     // ID: org.bluetooth.characteristic.cgm_session_run_time
    { 0x2AAC, "CGM Specific Ops Control Point" },           // ID: org.bluetooth.characteristic.cgm_specific_ops_control_point
    { 0x2AAD, "Indoor Positioning Configuration" },         // ID: org.bluetooth.characteristic.indoor_positioning_configuration
    { 0x2AAE, "Latitude" },                                 // ID: org.bluetooth.characteristic.latitude
    { 0x2AAF, "Longitude" },                                // ID: org.bluetooth.characteristic.longitude
    { 0x2AB0, "Local North Coordinate" },                   // ID: org.bluetooth.characteristic.local_north_coordinate
    { 0x2AB1, "Local East Coordinate" },                    // ID: org.bluetooth.characteristic.local_east_coordinate
    { 0x2AB2, "Floor Number" },                             // ID: org.bluetooth.characteristic.floor_number
    { 0x2AB3, "Altitude" },                                 // ID: org.bluetooth.characteristic.altitude
    { 0x2AB4, "Uncertainty" },                              // ID: org.bluetooth.characteristic.uncertainty
    { 0x2AB5, "Location Name" },                            // ID: org.bluetooth.characteristic.location_name
    { 0x2AB6, "URI" },                                      // ID: org.bluetooth.characteristic.uri
    { 0x2AB7, "HTTP Headers" },                             // ID: org.bluetooth.characteristic.http_headers
    { 0x2AB8, "HTTP Status Code" },                         // ID: org.bluetooth.characteristic.http_status_code
    { 0x2AB9, "HTTP Entity Body" },                         // ID: org.bluetooth.characteristic.http_entity_body
    { 0x2ABA, "HTTP Control Point" },                       // ID: org.bluetooth.characteristic.http_control_point
    { 0x2ABB, "HTTPS Security" },                           // ID: org.bluetooth.characteristic.https_security
    { 0x2ABC, "TDS Control Point" },                        // ID: org.bluetooth.characteristic.tds_control_point
    { 0x2ABD, "OTS Feature" },                              // ID: org.bluetooth.characteristic.ots_feature
    { 0x2ABE, "Object Name" },                              // ID: org.bluetooth.characteristic.object_name
    { 0x2ABF, "Object Type" },                              // ID: org.bluetooth.characteristic.object_type
    { 0x2AC0, "Object Size" },                              // ID: org.bluetooth.characteristic.object_size
    { 0x2AC1, "Object First-Created" },                     // ID: org.bluetooth.characteristic.object_first_created
    { 0x2AC2, "Object Last-Modified" },                     // ID: org.bluetooth.characteristic.object_last_modified
    { 0x2AC3, "Object ID" },                                // ID: org.bluetooth.characteristic.object_id
    { 0x2AC4, "Object Properties" },                        // ID: org.bluetooth.characteristic.object_properties
    { 0x2AC5, "Object Action Control Point" },              // ID: org.bluetooth.characteristic.object_action_control_point
    { 0x2AC6, "Object List Control Point" },                // ID: org.bluetooth.characteristic.object_list_control_point
    { 0x2AC7, "Object List Filter" },                       // ID: org.bluetooth.characteristic.object_list_filter
    { 0x2AC8, "Object Changed" },                           // ID: org.bluetooth.characteristic.object_changed
    { 0x2AC9, "Resolvable Private Address Only" },          // ID: org.bluetooth.characteristic.resolvable_private_address_only
    { 0x2ACC, "Fitness Machine Feature" },                  // ID: org.bluetooth.characteristic.fitness_machine_feature
    { 0x2ACD, "Treadmill Data" },                           // ID: org.bluetooth.characteristic.treadmill_data
    { 0x2ACE, "Cross Trainer Data" },                       // ID: org.bluetooth.characteristic.cross_trainer_data
    { 0x2ACF, "Step Climber Data" },                        // ID: org.bluetooth.characteristic.step_climber_data
    { 0x2AD0, "Stair Climber Data" },                       // ID: org.bluetooth.characteristic.stair_climber_data
    { 0x2AD1, "Rower Data" },                               // ID: org.bluetooth.characteristic.rower_data
    { 0x2AD2, "Indoor Bike Data" },                         // ID: org.bluetooth.characteristic.indoor_bike_data
    { 0x2AD3, "Training Status" },                          // ID: org.bluetooth.characteristic.training_status
    { 0x2AD4, "Supported Speed Range" },                    // ID: org.bluetooth.characteristic.supported_speed_range
    { 0x2AD5, "Supported Inclination Range" },              // ID: org.bluetooth.characteristic.supported_inclination_range
    { 0x2AD6, "Supported Resistance Level Range" },         // ID: org.bluetooth.characteristic.supported_resistance_level_range
    { 0x2AD7, "Supported Heart Rate Range" },               // ID: org.bluetooth.characteristic.supported_heart_rate_range
    { 0x2AD8, "Supported Power Range" },                    // ID: org.bluetooth.characteristic.supported_power_range
    { 0x2AD9, "Fitness Machine Control Point" },            // ID: org.bluetooth.characteristic.fitness_machine_control_point
    { 0x2ADA, "Fitness Machine Status" },                   // ID: org.bluetooth.characteristic.fitness_machine_status
    { 0x2ADB, "Mesh Provisioning Data In" },                // ID: org.bluetooth.characteristic.mesh_provisioning_data_in
    { 0x2ADC, "Mesh Provisioning Data Out" },               // ID: org.bluetooth.characteristic.mesh_provisioning_data_out
    { 0x2ADD, "Mesh Proxy Data In" },                       // ID: org.bluetooth.characteristic.mesh_proxy_data_in
    { 0x2ADE, "Mesh Proxy Data Out" },                      // ID: org.bluetooth.characteristic.mesh_proxy_data_out
    { 0x2AE0, "Average Current" },                          // ID: org.bluetooth.characteristic.average_current
    { 0x2AE1, "Average Voltage" },                          // ID: org.bluetooth.characteristic.average_voltage
    { 0x2AE2, "Boolean" },                                  // ID: org.bluetooth.characteristic.boolean
    { 0x2AE3, "Chromatic Distance from Planckian" },        // ID: org.bluetooth.characteristic.chromatic_distance_from_planckian
    { 0x2AE4, "Chromaticity Coordinates" },                 // ID: org.bluetooth.characteristic.chromaticity_coordinates
    { 0x2AE5, "Chromaticity in CCT and Duv Values" },       // ID: org.bluetooth.characteristic.chromaticity_in_cct_and_duv_values
    { 0x2AE6, "Chromaticity Tolerance" },                   // ID: org.bluetooth.characteristic.chromaticity_tolerance
    { 0x2AE7, "CIE 13.3-1995 Color Rendering Index" },      // ID: org.bluetooth.characteristic.cie_13_3_1995_color_rendering_index
    { 0x2AE8, "Coefficient" },                              // ID: org.bluetooth.characteristic.coefficient
    { 0x2AE9, "Correlated Color Temperature" },             // ID: org.bluetooth.characteristic.correlated_color_temperature
    { 0x2AEA, "Count 16" },                                 // ID: org.bluetooth.characteristic.count_16
    { 0x2AEB, "Count 24" },                                 // ID: org.bluetooth.characteristic.count_24
    { 0x2AEC, "Country Code" },                             // ID: org.bluetooth.characteristic.country_code
    { 0x2AED, "Date UTC" },                                 // ID: org.bluetooth.characteristic.date_utc
    { 0x2AEE, "Electric Current" },                         // ID: org.bluetooth.characteristic.electric_current
    { 0x2AEF, "Electric Current Range" },                   // ID: org.bluetooth.characteristic.electric_current_range
    { 0x2AF0, "Electric Current Specification" },           // ID: org.bluetooth.characteristic.electric_current_specification
    { 0x2AF1, "Electric Current Statistics" },              // ID: org.bluetooth.characteristic.electric_current_statistics
    { 0x2AF2, "Energy" },                                   // ID: org.bluetooth.characteristic.energy
    { 0x2AF3, "Energy in a Period of Day" },                // ID: org.bluetooth.characteristic.energy_in_a_period_of_day
    { 0x2AF4, "Event Statistics" },                         // ID: org.bluetooth.characteristic.event_statistics
    { 0x2AF5, "Fixed String 16" },                          // ID: org.bluetooth.characteristic.fixed_string_16
    { 0x2AF6, "Fixed String 24" },                          // ID: org.bluetooth.characteristic.fixed_string_24
    { 0x2AF7, "Fixed String 36" },                          // ID: org.bluetooth.characteristic.fixed_string_36
    { 0x2AF8, "Fixed String 8" },                           // ID: org.bluetooth.characteristic.fixed_string_8
    { 0x2AF9, "Generic Level" },                            // ID: org.bluetooth.characteristic.generic_level
    { 0x2AFA, "Global Trade Item Number" },                 // ID: org.bluetooth.characteristic.global_trade_item_number
    { 0x2AFB, "Illuminance" },                              // ID: org.bluetooth.characteristic.illuminance
    { 0x2AFC, "Luminous Efficacy" },                        // ID: org.bluetooth.characteristic.luminous_efficacy
    { 0x2AFD, "Luminous Energy" },                          // ID: org.bluetooth.characteristic.luminous_energy
    { 0x2AFE, "Luminous Exposure" },                        // ID: org.bluetooth.characteristic.luminous_exposure
    { 0x2AFF, "Luminous Flux" },                            // ID: org.bluetooth.characteristic.luminous_flux
    { 0x2B00, "Luminous Flux Range" },                      // ID: org.bluetooth.characteristic.luminous_flux_range
    { 0x2B01, "Luminous Intensity" },                       // ID: org.bluetooth.characteristic.luminous_intensity
    { 0x2B02, "Mass Flow" },                                // ID: org.bluetooth.characteristic.mass_flow
    { 0x2B03, "Perceived Lightness" },                      // ID: org.bluetooth.characteristic.perceived_lightness
    { 0x2B04, "Percentage 8" },                             // ID: org.bluetooth.characteristic.percentage_8
    { 0x2B05, "Power" },                                    // ID: org.bluetooth.characteristic.power
    { 0x2B06, "Power Specification" },                      // ID: org.bluetooth.characteristic.power_specification
    { 0x2B07, "Relative Runtime in a Current Range" },      // ID: org.bluetooth.characteristic.relative_runtime_in_a_current_range
    { 0x2B08, "Relative Runtime in a Generic Level Range" },// ID: org.bluetooth.characteristic.relative_runtime_in_a_generic_level_range
    { 0x2B09, "Relative Value in a Voltage Range" },        // ID: org.bluetooth.characteristic.relative_value_in_a_voltage_range
    { 0x2B0A, "Relative Value in an Illuminance Range" },   // ID: org.bluetooth.characteristic.relative_value_in_an_illuminance_range
    { 0x2B0B, "Relative Value in a Period of Day" },        // ID: org.bluetooth.characteristic.relative_value_in_a_period_of_day
    { 0x2B0C, "Relative Value in a Temperature Range" },    // ID: org.bluetooth.characteristic.relative_value_in_a_temperature_range
    { 0x2B0D, "Temperature 8" },                            // ID: org.bluetooth.characteristic.temperature_8
    { 0x2B0E, "Temperature 8 in a Period of Day" },         // ID: org.bluetooth.characteristic.temperature_8_in_a_period_of_day
    { 0x2B0F, "Temperature 8 Statistics" },                 // ID: org.bluetooth.characteristic.temperature_8_statistics
    { 0x2B10, "Temperature Range" },                        // ID: org.bluetooth.characteristic.temperature_range
    { 0x2B11, "Temperature Statistics" },                   // ID: org.bluetooth.characteristic.temperature_statistics
    { 0x2B12, "Time Decihour 8" },                          // ID: org.bluetooth.characteristic.time_decihour_8
    { 0x2B13, "Time Exponential 8" },                       // ID: org.bluetooth.characteristic.time_exponential_8
    { 0x2B14, "Time Hour 24" },                             // ID: org.bluetooth.characteristic.time_hour_24
    { 0x2B15, "Time Millisecond 24" },                      // ID: org.bluetooth.characteristic.time_millisecond_24
    { 0x2B16, "Time Second 16" },                           // ID: org.bluetooth.characteristic.time_second_16
    { 0x2B17, "Time Second 8" },                            // ID: org.bluetooth.characteristic.time_second_8
    { 0x2B18, "Voltage" },                                  // ID: org.bluetooth.characteristic.voltage
    { 0x2B19, "Voltage Specification" },                    // ID: org.bluetooth.characteristic.voltage_specification
    { 0x2B1A, "Voltage Statistics" },                       // ID: org.bluetooth.characteristic.voltage_statistics
    { 0x2B1B, "Volume Flow" },                              // ID: org.bluetooth.characteristic.volume_flow
    { 0x2B1C, "Chromaticity Coordinate" },                  // ID: org.bluetooth.characteristic.chromaticity_coordinate
    { 0x2B1D, "RC Feature" },                               // ID: org.bluetooth.characteristic.rc_feature
    { 0x2B1E, "RC Settings" },                              // ID: org.bluetooth.characteristic.rc_settings
    { 0x2B1F, "Reconnection Configuration Control Point" }, // ID: org.bluetooth.characteristic.reconnection_configuration_control_point
    { 0x2B20, "IDD Status Changed" },                       // ID: org.bluetooth.characteristic.idd_status_changed
    { 0x2B21, "IDD Status" },                               // ID: org.bluetooth.characteristic.idd_status
    { 0x2B22, "IDD Annunciation Status" },                  // ID: org.bluetooth.characteristic.idd_annunciation_status
    { 0x2B23, "IDD Features" },                             // ID: org.bluetooth.characteristic.idd_features
    { 0x2B24, "IDD Status Reader Control Point" },          // ID: org.bluetooth.characteristic.idd_status_reader_control_point
    { 0x2B25, "IDD Command Control Point" },                // ID: org.bluetooth.characteristic.idd_command_control_point
    { 0x2B26, "IDD Command Data" },                         // ID: org.bluetooth.characteristic.idd_command_data
    { 0x2B27, "IDD Record Access Control Point" },          // ID: org.bluetooth.characteristic.idd_record_access_control_point
    { 0x2B28, "IDD History Data" },                         // ID: org.bluetooth.characteristic.idd_history_data
    { 0x2B29, "Client Supported Features" },                // ID: org.bluetooth.characteristic.client_supported_features
    { 0x2B2A, "Database Hash" },                            // ID: org.bluetooth.characteristic.database_hash
    { 0x2B2B, "BSS Control Point" },                        // ID: org.bluetooth.characteristic.bss_control_point
    { 0x2B2C, "BSS Response" },                             // ID: org.bluetooth.characteristic.bss_response
    { 0x2B2D, "Emergency ID" },                             // ID: org.bluetooth.characteristic.emergency_id
    { 0x2B2E, "Emergency Text" },                           // ID: org.bluetooth.characteristic.emergency_text
    { 0x2B2F, "ACS Status" },                               // ID: org.bluetooth.characteristic.acs_status
    { 0x2B30, "ACS Data In" },                              // ID: org.bluetooth.characteristic.acs_data_in
    { 0x2B31, "ACS Data Out Notify" },                      // ID: org.bluetooth.characteristic.acs_data_out_notify
    { 0x2B32, "ACS Data Out Indicate" },                    // ID: org.bluetooth.characteristic.acs_data_out_indicate
    { 0x2B33, "ACS Control Point" },                        // ID: org.bluetooth.characteristic.acs_control_point
    { 0x2B34, "Enhanced Blood Pressure Measurement" },      // ID: org.bluetooth.characteristic.enhanced_blood_pressure_measurement
    { 0x2B35, "Enhanced Intermediate Cuff Pressure" },      // ID: org.bluetooth.characteristic.enhanced_intermediate_cuff_pressure
    { 0x2B36, "Blood Pressure Record" },                    // ID: org.bluetooth.characteristic.blood_pressure_record
    { 0x2B37, "Registered User" },                          // ID: org.bluetooth.characteristic.registered_user
    { 0x2B38, "BR-EDR Handover Data" },                     // ID: org.bluetooth.characteristic.br_edr_handover_data
    { 0x2B39, "Bluetooth SIG Data" },                       // ID: org.bluetooth.characteristic.bluetooth_sig_data
    { 0x2B3A, "Server Supported Features" },                // ID: org.bluetooth.characteristic.server_supported_features
    { 0x2B3B, "Physical Activity Monitor Features" },       // ID: org.bluetooth.characteristic.physical_activity_monitor_features
    { 0x2B3C, "General Activity Instantaneous Data" },      // ID: org.bluetooth.characteristic.general_activity_instantaneous_data
    { 0x2B3D, "General Activity Summary Data" },            // ID: org.bluetooth.characteristic.general_activity_summary_data
    { 0x2B3E, "CardioRespiratory Activity Instantaneous Data" },// ID: org.bluetooth.characteristic.cardiorespiratory_activity_instantaneous_data
    { 0x2B3F, "CardioRespiratory Activity Summary Data" },  // ID: org.bluetooth.characteristic.cardiorespiratory_activity_summary_data
    { 0x2B40, "Step Counter Activity Summary Data" },       // ID: org.bluetooth.characteristic.step_counter_activity_summary_data
    { 0x2B41, "Sleep Activity Instantaneous Data" },        // ID: org.bluetooth.characteristic.sleep_activity_instantaneous_data
    { 0x2B42, "Sleep Activity Summary Data" },              // ID: org.bluetooth.characteristic.sleep_activity_summary_data
    { 0x2B43, "Physical Activity Monitor Control Point" },  // ID: org.bluetooth.characteristic.physical_activity_monitor_control_point
    { 0x2B44, "Physical Activity Current Session" },        // ID: org.bluetooth.characteristic.physical_activity_current_session
    { 0x2B45, "Physical Activity Session Descriptor" },     // ID: org.bluetooth.characteristic.physical_activity_session_descriptor
    { 0x2B46, "Preferred Units" },                          // ID: org.bluetooth.characteristic.preferred_units
    { 0x2B47, "High Resolution Height" },                   // ID: org.bluetooth.characteristic.high_resolution_height
    { 0x2B48, "Middle Name" },                              // ID: org.bluetooth.characteristic.middle_name
    { 0x2B49, "Stride Length" },                            // ID: org.bluetooth.characteristic.stride_length
    { 0x2B4A, "Handedness" },                               // ID: org.bluetooth.characteristic.handedness
    { 0x2B4B, "Device Wearing Position" },                  // ID: org.bluetooth.characteristic.device_wearing_position
    { 0x2B4C, "Four Zone Heart Rate Limits" },              // ID: org.bluetooth.characteristic.four_zone_heart_rate_limits
    { 0x2B4D, "High Intensity Exercise Threshold" },        // ID: org.bluetooth.characteristic.high_intensity_exercise_threshold
    { 0x2B4E, "Activity Goal" },                            // ID: org.bluetooth.characteristic.activity_goal
    { 0x2B4F, "Sedentary Interval Notification" },          // ID: org.bluetooth.characteristic.sedentary_interval_notification
    { 0x2B50, "Caloric Intake" },                           // ID: org.bluetooth.characteristic.caloric_intake
    { 0x2B51, "TMAP Role" },                                // ID: org.bluetooth.characteristic.tmap_role
    { 0x2B77, "Audio Input State" },                        // ID: org.bluetooth.characteristic.audio_input_state
    { 0x2B78, "Gain Settings Attribute" },                  // ID: org.bluetooth.characteristic.gain_settings_attribute
    { 0x2B79, "Audio Input Type" },                         // ID: org.bluetooth.characteristic.audio_input_type
    { 0x2B7A, "Audio Input Status" },                       // ID: org.bluetooth.characteristic.audio_input_status
    { 0x2B7B, "Audio Input Control Point" },                // ID: org.bluetooth.characteristic.audio_input_control_point
    { 0x2B7C, "Audio Input Description" },                  // ID: org.bluetooth.characteristic.audio_input_description
    { 0x2B7D, "Volume State" },                             // ID: org.bluetooth.characteristic.volume_state
    { 0x2B7E, "Volume Control Point" },                     // ID: org.bluetooth.characteristic.volume_control_point
    { 0x2B7F, "Volume Flags" },                             // ID: org.bluetooth.characteristic.volume_flags
    { 0x2B80, "Volume Offset State" },                      // ID: org.bluetooth.characteristic.volume_offset_state
    { 0x2B81, "Audio Location" },                           // ID: org.bluetooth.characteristic.audio_location
    { 0x2B82, "Volume Offset Control Point" },              // ID: org.bluetooth.characteristic.volume_offset_control_point
    { 0x2B83, "Audio Output Description" },                 // ID: org.bluetooth.characteristic.audio_output_description
    { 0x2B84, "Set Identity Resolving Key" },               // ID: org.bluetooth.characteristic.set_identity_resolving_key
    { 0x2B85, "Coordinated Set Size" },                     // ID: org.bluetooth.characteristic.size_characteristic
    { 0x2B86, "Set Member Lock" },                          // ID: org.bluetooth.characteristic.lock_characteristic
    { 0x2B87, "Set Member Rank" },                          // ID: org.bluetooth.characteristic.rank_characteristic
    { 0x2B88, "Encrypted Data Key Material" },              // ID: org.bluetooth.characteristic.encrypted_data_key_material
    { 0x2B89, "Apparent Energy 32" },                       // ID: org.bluetooth.characteristic.apparent_energy_32
    { 0x2B8A, "Apparent Power" },                           // ID: org.bluetooth.characteristic.apparent_power
    { 0x2B8B, "Live Health Observations" },                 // ID: org.bluetooth.characteristic.live_health_observations
    { 0x2B8C, "CO\textsubscript{2} Concentration" },        // ID: org.bluetooth.characteristic.co2_concentration
    { 0x2B8D, "Cosine of the Angle" },                      // ID: org.bluetooth.characteristic.cosine_of_the_angle
    { 0x2B8E, "Device Time Feature" },                      // ID: org.bluetooth.characteristic.device_time_feature
    { 0x2B8F, "Device Time Parameters" },                   // ID: org.bluetooth.characteristic.device_time_parameters
    { 0x2B90, "Device Time" },                              // ID: org.bluetooth.characteristic.device_time
    { 0x2B91, "Device Time Control Point" },                // ID: org.bluetooth.characteristic.device_time_control_point
    { 0x2B92, "Time Change Log Data" },                     // ID: org.bluetooth.characteristic.time_change_log_data
    { 0x2B93, "Media Player Name" },                        // ID: org.bluetooth.characteristic.media_player_name
    { 0x2B94, "Media Player Icon Object ID" },              // ID: org.bluetooth.characteristic.media_player_icon_object_id
    { 0x2B95, "Media Player Icon URL" },                    // ID: org.bluetooth.characteristic.media_player_icon_url
    { 0x2B96, "Track Changed" },                            // ID: org.bluetooth.characteristic.track_changed
    { 0x2B97, "Track Title" },                              // ID: org.bluetooth.characteristic.track_title
    { 0x2B98, "Track Duration" },                           // ID: org.bluetooth.characteristic.track_duration
    { 0x2B99, "Track Position" },                           // ID: org.bluetooth.characteristic.track_position
    { 0x2B9A, "Playback Speed" },                           // ID: org.bluetooth.characteristic.playback_speed
    { 0x2B9B, "Seeking Speed" },                            // ID: org.bluetooth.characteristic.seeking_speed
    { 0x2B9C, "Current Track Segments Object ID" },         // ID: org.bluetooth.characteristic.current_track_segments_object_id
    { 0x2B9D, "Current Track Object ID" },                  // ID: org.bluetooth.characteristic.current_track_object_id
    { 0x2B9E, "Next Track Object ID" },                     // ID: org.bluetooth.characteristic.next_track_object_id
    { 0x2B9F, "Parent Group Object ID" },                   // ID: org.bluetooth.characteristic.parent_group_object_id
    { 0x2BA0, "Current Group Object ID" },                  // ID: org.bluetooth.characteristic.current_group_object_id
    { 0x2BA1, "Playing Order" },                            // ID: org.bluetooth.characteristic.playing_order
    { 0x2BA2, "Playing Orders Supported" },                 // ID: org.bluetooth.characteristic.playing_orders_supported
    { 0x2BA3, "Media State" },                              // ID: org.bluetooth.characteristic.media_state
    { 0x2BA4, "Media Control Point" },                      // ID: org.bluetooth.characteristic.media_control_point
    { 0x2BA5, "Media Control Point Opcodes Supported" },    // ID: org.bluetooth.characteristic.media_control_point_opcodes_supported
    { 0x2BA6, "Search Results Object ID" },                 // ID: org.bluetooth.characteristic.search_results_object_id
    { 0x2BA7, "Search Control Point" },                     // ID: org.bluetooth.characteristic.search_control_point
    { 0x2BA8, "Energy 32" },                                // ID: org.bluetooth.characteristic.energy_32
    { 0x2BAD, "Constant Tone Extension Enable" },           // ID: org.bluetooth.characteristic.constant_tone_extension_enable
    { 0x2BAE, "Advertising Constant Tone Extension Minimum Length" },// ID: org.bluetooth.characteristic.advertising_constant_tone_extension_minimum_length
    { 0x2BAF, "Advertising Constant Tone Extension Minimum Transmit Count" },// ID: org.bluetooth.characteristic.advertising_constant_tone_extension_minimum_transmit_count
    { 0x2BB0, "Advertising Constant Tone Extension Transmit Duration" },// ID: org.bluetooth.characteristic.advertising_constant_tone_extension_transmit_duration
    { 0x2BB1, "Advertising Constant Tone Extension Interval" },// ID: org.bluetooth.characteristic.advertising_constant_tone_extension_interval
    { 0x2BB2, "Advertising Constant Tone Extension PHY" },  // ID: org.bluetooth.characteristic.advertising_constant_tone_extension_phy
    { 0x2BB3, "Bearer Provider Name" },                     // ID: org.bluetooth.characteristic.bearer_provider_name
    { 0x2BB4, "Bearer UCI" },                               // ID: org.bluetooth.characteristic.bearer_uci
    { 0x2BB5, "Bearer Technology" },                        // ID: org.bluetooth.characteristic.bearer_technology
    { 0x2BB6, "Bearer URI Schemes Supported List" },        // ID: org.bluetooth.characteristic.bearer_uri_schemes_supported_list
    { 0x2BB7, "Bearer Signal Strength" },                   // ID: org.bluetooth.characteristic.bearer_signal_strength
    { 0x2BB8, "Bearer Signal Strength Reporting Interval" },// ID: org.bluetooth.characteristic.bearer_signal_strength_reporting_interval
    { 0x2BB9, "Bearer List Current Calls" },                // ID: org.bluetooth.characteristic.bearer_list_current_calls
    { 0x2BBA, "Content Control ID" },                       // ID: org.bluetooth.characteristic.content_control_id
    { 0x2BBB, "Status Flags" },                             // ID: org.bluetooth.characteristic.status_flags
    { 0x2BBC, "Incoming Call Target Bearer URI" },          // ID: org.bluetooth.characteristic.incoming_call_target_bearer_uri
    { 0x2BBD, "Call State" },                               // ID: org.bluetooth.characteristic.call_state
    { 0x2BBE, "Call Control Point" },                       // ID: org.bluetooth.characteristic.call_control_point
    { 0x2BBF, "Call Control Point Optional Opcodes" },      // ID: org.bluetooth.characteristic.call_control_point_optional_opcodes
    { 0x2BC0, "Termination Reason" },                       // ID: org.bluetooth.characteristic.termination_reason
    { 0x2BC1, "Incoming Call" },                            // ID: org.bluetooth.characteristic.incoming_call
    { 0x2BC2, "Call Friendly Name" },                       // ID: org.bluetooth.characteristic.call_friendly_name
    { 0x2BC3, "Mute" },                                     // ID: org.bluetooth.characteristic.mute
    { 0x2BC4, "Sink ASE" },                                 // ID: org.bluetooth.characteristic.sink_ase
    { 0x2BC5, "Source ASE" },                               // ID: org.bluetooth.characteristic.source_ase
    { 0x2BC6, "ASE Control Point" },                        // ID: org.bluetooth.characteristic.ase_control_point
    { 0x2BC7, "Broadcast Audio Scan Control Point" },       // ID: org.bluetooth.characteristic.broadcast_audio_scan_control_point
    { 0x2BC8, "Broadcast Receive State" },                  // ID: org.bluetooth.characteristic.broadcast_receive_state
    { 0x2BC9, "Sink PAC" },                                 // ID: org.bluetooth.characteristic.sink_pac
    { 0x2BCA, "Sink Audio Locations" },                     // ID: org.bluetooth.characteristic.sink_audio_locations
    { 0x2BCB, "Source PAC" },                               // ID: org.bluetooth.characteristic.source_pac
    { 0x2BCC, "Source Audio Locations" },                   // ID: org.bluetooth.characteristic.source_audio_locations
    { 0x2BCD, "Available Audio Contexts" },                 // ID: org.bluetooth.characteristic.available_audio_contexts
    { 0x2BCE, "Supported Audio Contexts" },                 // ID: org.bluetooth.characteristic.supported_audio_contexts
    { 0x2BCF, "Ammonia Concentration" },                    // ID: org.bluetooth.characteristic.ammonia_concentration
    { 0x2BD0, "Carbon Monoxide Concentration" },            // ID: org.bluetooth.characteristic.carbon_monoxide_concentration
    { 0x2BD1, "Methane Concentration" },                    // ID: org.bluetooth.characteristic.methane_concentration
    { 0x2BD2, "Nitrogen Dioxide Concentration" },           // ID: org.bluetooth.characteristic.nitrogen_dioxide_concentration
    { 0x2BD3, "Non-Methane Volatile Organic Compounds Concentration" },// ID: org.bluetooth.characteristic.non-methane_volatile_organic_compounds_concentration
    { 0x2BD4, "Ozone Concentration" },                      // ID: org.bluetooth.characteristic.ozone_concentration
    { 0x2BD5, "Particulate Matter - PM1 Concentration" },   // ID: org.bluetooth.characteristic.particulate_matter_pm1_concentration
    { 0x2BD6, "Particulate Matter - PM2.5 Concentration" }, // ID: org.bluetooth.characteristic.particulate_matter_pm2_5_concentration
    { 0x2BD7, "Particulate Matter - PM10 Concentration" },  // ID: org.bluetooth.characteristic.particulate_matter_pm10_concentration
    { 0x2BD8, "Sulfur Dioxide Concentration" },             // ID: org.bluetooth.characteristic.sulfur_dioxide_concentration
    { 0x2BD9, "Sulfur Hexafluoride Concentration" },        // ID: org.bluetooth.characteristic.sulfur_hexafluoride_concentration
    { 0x2BDA, "Hearing Aid Features" },                     // ID: org.bluetooth.characteristic.hearing_aid_features
    { 0x2BDB, "Hearing Aid Preset Control Point" },         // ID: org.bluetooth.characteristic.hearing_aid_preset_control_point
    { 0x2BDC, "Active Preset Index" },                      // ID: org.bluetooth.characteristic.active_preset_index
    { 0x2BDD, "Stored Health Observations" },               // ID: org.bluetooth.characteristic.stored_health_observations
    { 0x2BDE, "Fixed String 64" },                          // ID: org.bluetooth.characteristic.fixed_string_64
    { 0x2BDF, "High Temperature" },                         // ID: org.bluetooth.characteristic.high_temperature
    { 0x2BE0, "High Voltage" },                             // ID: org.bluetooth.characteristic.high_voltage
    { 0x2BE1, "Light Distribution" },                       // ID: org.bluetooth.characteristic.light_distribution
    { 0x2BE2, "Light Output" },                             // ID: org.bluetooth.characteristic.light_output
    { 0x2BE3, "Light Source Type" },                        // ID: org.bluetooth.characteristic.light_source_type
    { 0x2BE4, "Noise" },                                    // ID: org.bluetooth.characteristic.noise
    { 0x2BE5, "Relative Runtime in a Correlated Color Temperature Range" },// ID: org.bluetooth.characteristic.relative_runtime_in_a_correlated_color_temperature_range
    { 0x2BE6, "Time Second 32" },                           // ID: org.bluetooth.characteristic.time_second_32
    { 0x2BE7, "VOC Concentration" },                        // ID: org.bluetooth.characteristic.voc_concentration
    { 0x2BE8, "Voltage Frequency" },                        // ID: org.bluetooth.characteristic.voltage_frequency
    { 0x2BE9, "Battery Critical Status" },                  // ID: org.bluetooth.characteristic.battery_critical_status
    { 0x2BEA, "Battery Health Status" },                    // ID: org.bluetooth.characteristic.battery_health_status
    { 0x2BEB, "Battery Health Information" },               // ID: org.bluetooth.characteristic.battery_health_information
    { 0x2BEC, "Battery Information" },                      // ID: org.bluetooth.characteristic.battery_information
    { 0x2BED, "Battery Level Status" },                     // ID: org.bluetooth.characteristic.battery_level_status
    { 0x2BEE, "Battery Time Status" },                      // ID: org.bluetooth.characteristic.battery_time_status
    { 0x2BEF, "Estimated Service Date" },                   // ID: org.bluetooth.characteristic.estimated_service_date
    { 0x2BF0, "Battery Energy Status" },                    // ID: org.bluetooth.characteristic.battery_energy_status
    { 0x2BF1, "Observation Schedule Changed" },             // ID: org.bluetooth.characteristic.observation_schedule_changed
    { 0x2BF2, "Current Elapsed Time" },                     // ID: org.bluetooth.characteristic.current_elapsed_time
    { 0x2BF3, "Health Sensor Features" },                   // ID: org.bluetooth.characteristic.health_sensor_features
    { 0x2BF4, "GHS Control Point" },                        // ID: org.bluetooth.characteristic.ghs_control_point
    { 0x2BF5, "LE GATT Security Levels" },                  // ID: org.bluetooth.characteristic.le_gatt_security_levels
    { 0x2BF6, "ESL Address" },                              // ID: org.bluetooth.characteristic.esl_address
    { 0x2BF7, "AP Sync Key Material" },                     // ID: org.bluetooth.characteristic.ap_sync_key_material
    { 0x2BF8, "ESL Response Key Material" },                // ID: org.bluetooth.characteristic.esl_response_key_material
    { 0x2BF9, "ESL Current Absolute Time" },                // ID: org.bluetooth.characteristic.esl_current_absolute_time
    { 0x2BFA, "ESL Display Information" },                  // ID: org.bluetooth.characteristic.esl_display_information
    { 0x2BFB, "ESL Image Information" },                    // ID: org.bluetooth.characteristic.esl_image_information
    { 0x2BFC, "ESL Sensor Information" },                   // ID: org.bluetooth.characteristic.esl_sensor_information
    { 0x2BFD, "ESL LED Information" },                      // ID: org.bluetooth.characteristic.esl_led_information
    { 0x2BFE, "ESL Control Point" },                        // ID: org.bluetooth.characteristic.esl_control_point
    { 0x2BFF, "UDI for Medical Devices" },                  // ID: org.bluetooth.characteristic.medical_devices
    { 0x2C00, "GMAP Role" },                                // ID: org.bluetooth.characteristic.gmap_role
    { 0x2C01, "UGG Features" },                             // ID: org.bluetooth.characteristic.ugg_features
    { 0x2C02, "UGT Features" },                             // ID: org.bluetooth.characteristic.ugt_features
    { 0x2C03, "BGS Features" },                             // ID: org.bluetooth.characteristic.bgs_features
    { 0x2C04, "BGR Features" },                             // ID: org.bluetooth.characteristic.bgr_features
    { 0x2C05, "Percentage 8 Steps" },                       // ID: org.bluetooth.characteristic.percentage_8_steps
    { 0x2C06, "Acceleration" },                             // ID: org.bluetooth.characteristic.acceleration
    { 0x2C07, "Force" },                                    // ID: org.bluetooth.characteristic.force
    { 0x2C08, "Linear Position" },                          // ID: org.bluetooth.characteristic.linear_position
    { 0x2C09, "Rotational Speed" },                         // ID: org.bluetooth.characteristic.rotational_speed
    { 0x2C0A, "Length" },                                   // ID: org.bluetooth.characteristic.length
    { 0x2C0B, "Torque" },                                   // ID: org.bluetooth.characteristic.torque
    { 0x2C0C, "IMD Status" },                               // ID: org.bluetooth.characteristic.imd_status
    { 0x2C0D, "IMDS Descriptor Value Changed" },            // ID: org.bluetooth.characteristic.imds_descriptor_value_changed
    { 0x2C0E, "First Use Date" },                           // ID: org.bluetooth.characteristic.first_use_date
    { 0x2C0F, "Life Cycle Data" },                          // ID: org.bluetooth.characteristic.life_cycle_data
    { 0x2C10, "Work Cycle Data" },                          // ID: org.bluetooth.characteristic.work_cycle_data
    { 0x2C11, "Service Cycle Data" },                       // ID: org.bluetooth.characteristic.service_cycle_data
    { 0x2C12, "IMD Control" },                              // ID: org.bluetooth.characteristic.imd_control
    { 0x2C13, "IMD Historical Data" },                      // ID: org.bluetooth.characteristic.imd_historical_data
    { 0x2C14, "RAS Features" },                             // ID: org.bluetooth.characteristic.ras_features
    { 0x2C15, "Real-time Ranging Data" },                   // ID: org.bluetooth.characteristic.real-time_ranging_data
    { 0x2C16, "On-demand Ranging Data" },                   // ID: org.bluetooth.characteristic.on-demand_ranging_data
    { 0x2C17, "RAS Control Point" },                        // ID: org.bluetooth.characteristic.ras_control_point
    { 0x2C18, "Ranging Data Ready" },                       // ID: org.bluetooth.characteristic.ranging_data_ready
    { 0x2C19, "Ranging Data Overwritten" },                 // ID: org.bluetooth.characteristic.ranging_data_overwritten
};

std::string get_characteristic_uuids_name(uint16_t code)
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