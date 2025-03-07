#include "service_class.hpp"

#include <stddef.h>
#include <sstream>

typedef struct {
    uint16_t value;
    const char* name;
} KeyValue;

static const KeyValue data[] = {
    { 0x1000, "ServiceDiscoveryServerServiceClassID" },     // ID: org.bluetooth.service_class.service_discovery_server
    { 0x1001, "BrowseGroupDescriptorServiceClassID" },      // ID: org.bluetooth.service_class.browse_group_descriptor
    { 0x1101, "SerialPort" },                               // ID: org.bluetooth.profile.serial_port
    { 0x1102, "LANAccessUsingPPP" },                        // ID: org.bluetooth.profile.lan_access
    { 0x1103, "Dial-Up Networking" },                       // ID: org.bluetooth.profile.dial_up_networking
    { 0x1104, "IrMCSync" },                                 // ID: org.bluetooth.profile.synchronization_profile
    { 0x1105, "OBEXObjectPush" },                           // ID: org.bluetooth.profile.object_push
    { 0x1106, "OBEX File Transfer" },                       // ID: org.bluetooth.profile.file_transfer_profile
    { 0x1107, "IrMCSyncCommand" },                          // ID: org.bluetooth.profile.synchronization_command
    { 0x1108, "Headset" },                                  // ID: org.bluetooth.profile.headset
    { 0x1109, "CordlessTelephony" },                        // ID: org.bluetooth.profile.cordless_telephony
    { 0x110A, "Audio Source" },                             // ID: org.bluetooth.profile.a2dp_audio_source
    { 0x110B, "Audio Sink" },                               // ID: org.bluetooth.profile.a2dp_audio_sink
    { 0x110C, "A/V Remote Control Target" },                // ID: org.bluetooth.profile.avrcp_remote_control_target
    { 0x110D, "Advanced Audio Distribution" },              // ID: org.bluetooth.profile.a2dp
    { 0x110E, "A/V Remote Control" },                       // ID: org.bluetooth.profile.avrcp_remote_control_remote_control
    { 0x110F, "A/V Remote Control Controller" },            // ID: org.bluetooth.profile.avrcp_remote_control_remote_control_controller
    { 0x1110, "Intercom" },                                 // ID: org.bluetooth.profile.intercom
    { 0x1111, "Fax" },                                      // ID: org.bluetooth.profile.fax
    { 0x1112, "Headset Audio Gateway" },                    // ID: org.bluetooth.profile.headset_audio_gateway
    { 0x1113, "WAP" },                                      // ID: org.bluetooth.profile.wap
    { 0x1114, "WAP_CLIENT" },                               // ID: org.bluetooth.profile.wap_client
    { 0x1115, "PANU" },                                     // ID: org.bluetooth.profile.pan_user
    { 0x1116, "NAP" },                                      // ID: org.bluetooth.profile.pan_network_access_point
    { 0x1117, "GN" },                                       // ID: org.bluetooth.profile.pan_gn
    { 0x1118, "DirectPrinting" },                           // ID: org.bluetooth.profile.bpp_direct_printing
    { 0x1119, "ReferencePrinting" },                        // ID: org.bluetooth.profile.bpp_reference_printing
    { 0x111A, "Imaging" },                                  // ID: org.bluetooth.profile.bip_imaging
    { 0x111B, "Imaging Responder" },                        // ID: org.bluetooth.profile.bip_imaging_responder
    { 0x111C, "Imaging Automatic Archive" },                // ID: org.bluetooth.profile.bip_imaging_automatic_archive
    { 0x111D, "Imaging Referenced Objects" },               // ID: org.bluetooth.profile.bip_imaging_referenced_objects
    { 0x111E, "Hands-Free" },                               // ID: org.bluetooth.profile.hands_free
    { 0x111F, "AG Hands-Free" },                            // ID: org.bluetooth.service_class.ag_hands_free
    { 0x1120, "DirectPrintingReferencedObjectsService" },   // ID: org.bluetooth.service_class.bpp_direct_printing_referenced_objects
    { 0x1121, "ReflectedUI" },                              // ID: org.bluetooth.service_class.bpp_reflected_ui
    { 0x1122, "BasicPrinting" },                            // ID: org.bluetooth.profile.bpp
    { 0x1123, "PrintingStatus" },                           // ID: org.bluetooth.service_class.bpp_printing_status
    { 0x1124, "HID" },                                      // ID: org.bluetooth.profile.hid
    { 0x1125, "HardcopyCableReplacement" },                 // ID: org.bluetooth.profile.hcrp
    { 0x1126, "HCR_Print" },                                // ID: org.bluetooth.service_class.hcr_print
    { 0x1127, "HCR_Scan" },                                 // ID: org.bluetooth.service_class.hcr_scan
    { 0x1128, "Common_ISDN_Access" },                       // ID: org.bluetooth.service_class.isdn
    { 0x112D, "SIM Access" },                               // ID: org.bluetooth.profile.sap
    { 0x112E, "Phonebook Access Client" },                  // ID: org.bluetooth.service_class.phone_book_access_client
    { 0x112F, "Phonebook Access Server" },                  // ID: org.bluetooth.service_class.phone_book_access_server
    { 0x1130, "Phonebook Access Profile" },                 // ID: org.bluetooth.profile.phone_book_access_profile
    { 0x1131, "Headset - HS" },                             // ID: org.bluetooth.service_class.headset_hs
    { 0x1132, "Message Access Server" },                    // ID: org.bluetooth.service_class.map_server
    { 0x1133, "Message Notification Server" },              // ID: org.bluetooth.service_class.map_notification_server
    { 0x1134, "Message Access Profile" },                   // ID: org.bluetooth.profile.message_access_profile
    { 0x1135, "GNSS" },                                     // ID: org.bluetooth.profile.global_navigiation_satellite_system
    { 0x1136, "GNSS_Server" },                              // ID: org.bluetooth.service_class.global_navigiation_satellite_system_server
    { 0x1137, "3D Display" },                               // ID: org.bluetooth.service_class.3d_display
    { 0x1138, "3D Glasses" },                               // ID: org.bluetooth.service_class.3d_glasses
    { 0x1139, "3D Synch Profile" },                         // ID: org.bluetooth.profile.3d_synch_profile
    { 0x113A, "Multi Profile Specification" },              // ID: org.bluetooth.profile.multi_profile_specification
    { 0x113B, "MPS" },                                      // ID: org.bluetooth.service_class.multi_profile_service
    { 0x113C, "CTN Access Service" },                       // ID: org.bluetooth.service_class.calendar_task_note_access
    { 0x113D, "CTN Notification Service" },                 // ID: org.bluetooth.service_class.calendar_task_note_notification
    { 0x113E, "Calendar Tasks and Notes Profile" },         // ID: org.bluetooth.profile.calendar_task_notes_profile
    { 0x1200, "PnPInformation" },                           // ID: org.bluetooth.service_class.device_identification
    { 0x1201, "Generic Networking" },                       // ID: org.bluetooth.service_class.generic_networking
    { 0x1202, "GenericFileTransfer" },                      // ID: org.bluetooth.service_class.generic_file_transfer
    { 0x1203, "Generic Audio" },                            // ID: org.bluetooth.service_class.generic_audio
    { 0x1204, "GenericTelephony" },                         // ID: org.bluetooth.service_class.generic_telephony
    { 0x1205, "UPNP_Service" },                             // ID: org.bluetooth.service_class.enhanced_service_discovery_service
    { 0x1206, "UPNP_IP_Service" },                          // ID: org.bluetooth.service_class.enhanced_service_discovery_ip_service
    { 0x1300, "ESDP_UPNP_IP_PAN" },                         // ID: org.bluetooth.service_class.enhanced_service_discovery_ip_pan_service
    { 0x1301, "ESDP_UPNP_IP_LAP" },                         // ID: org.bluetooth.service_class.enhanced_service_discovery_ip_lap_service
    { 0x1302, "ESDP_UPNP_L2CAP" },                          // ID: org.bluetooth.service_class.enhanced_service_discovery_l2cap_service
    { 0x1303, "Video Source" },                             // ID: org.bluetooth.service_class.video_distribution_source
    { 0x1304, "Video Sink" },                               // ID: org.bluetooth.service_class.video_distribution_sink
    { 0x1305, "Video Distribution" },                       // ID: org.bluetooth.profile.video_distribution
    { 0x1400, "HDP" },                                      // ID: org.bluetooth.profile.health_device
    { 0x1401, "HDP Source" },                               // ID: org.bluetooth.service_class.health_device_source
    { 0x1402, "HDP Sink" },                                 // ID: org.bluetooth.service_class.health_device_sink
};

std::string get_service_class_name(uint16_t code)
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