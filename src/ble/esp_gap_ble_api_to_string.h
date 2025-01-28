
#ifndef __ESP_GAP_BLE_API_TO_STRING_H__
#define __ESP_GAP_BLE_API_TO_STRING_H__

#include "esp_gap_ble_api.h"


#ifdef __cplusplus
extern "C" {
#endif


const char* enum_to_string(esp_ble_adv_data_type type) {
    switch (type) {
        case ESP_BLE_AD_TYPE_FLAG: return "ESP_BLE_AD_TYPE_FLAG";
        case ESP_BLE_AD_TYPE_16SRV_PART: return "ESP_BLE_AD_TYPE_16SRV_PART";
        case ESP_BLE_AD_TYPE_16SRV_CMPL: return "ESP_BLE_AD_TYPE_16SRV_CMPL";
        case ESP_BLE_AD_TYPE_32SRV_PART: return "ESP_BLE_AD_TYPE_32SRV_PART";
        case ESP_BLE_AD_TYPE_32SRV_CMPL: return "ESP_BLE_AD_TYPE_32SRV_CMPL";
        case ESP_BLE_AD_TYPE_128SRV_PART: return "ESP_BLE_AD_TYPE_128SRV_PART";
        case ESP_BLE_AD_TYPE_128SRV_CMPL: return "ESP_BLE_AD_TYPE_128SRV_CMPL";
        case ESP_BLE_AD_TYPE_NAME_SHORT: return "ESP_BLE_AD_TYPE_NAME_SHORT";
        case ESP_BLE_AD_TYPE_NAME_CMPL: return "ESP_BLE_AD_TYPE_NAME_CMPL";
        case ESP_BLE_AD_TYPE_TX_PWR: return "ESP_BLE_AD_TYPE_TX_PWR";
        case ESP_BLE_AD_TYPE_DEV_CLASS: return "ESP_BLE_AD_TYPE_DEV_CLASS";
        case ESP_BLE_AD_TYPE_SM_TK: return "ESP_BLE_AD_TYPE_SM_TK";
        case ESP_BLE_AD_TYPE_SM_OOB_FLAG: return "ESP_BLE_AD_TYPE_SM_OOB_FLAG";
        case ESP_BLE_AD_TYPE_INT_RANGE: return "ESP_BLE_AD_TYPE_INT_RANGE";
        case ESP_BLE_AD_TYPE_SOL_SRV_UUID: return "ESP_BLE_AD_TYPE_SOL_SRV_UUID";
        case ESP_BLE_AD_TYPE_128SOL_SRV_UUID: return "ESP_BLE_AD_TYPE_128SOL_SRV_UUID";
        case ESP_BLE_AD_TYPE_SERVICE_DATA: return "ESP_BLE_AD_TYPE_SERVICE_DATA";
        case ESP_BLE_AD_TYPE_PUBLIC_TARGET: return "ESP_BLE_AD_TYPE_PUBLIC_TARGET";
        case ESP_BLE_AD_TYPE_RANDOM_TARGET: return "ESP_BLE_AD_TYPE_RANDOM_TARGET";
        case ESP_BLE_AD_TYPE_APPEARANCE: return "ESP_BLE_AD_TYPE_APPEARANCE";
        case ESP_BLE_AD_TYPE_ADV_INT: return "ESP_BLE_AD_TYPE_ADV_INT";
        case ESP_BLE_AD_TYPE_LE_DEV_ADDR: return "ESP_BLE_AD_TYPE_LE_DEV_ADDR";
        case ESP_BLE_AD_TYPE_LE_ROLE: return "ESP_BLE_AD_TYPE_LE_ROLE";
        case ESP_BLE_AD_TYPE_SPAIR_C256: return "ESP_BLE_AD_TYPE_SPAIR_C256";
        case ESP_BLE_AD_TYPE_SPAIR_R256: return "ESP_BLE_AD_TYPE_SPAIR_R256";
        case ESP_BLE_AD_TYPE_32SOL_SRV_UUID: return "ESP_BLE_AD_TYPE_32SOL_SRV_UUID";
        case ESP_BLE_AD_TYPE_32SERVICE_DATA: return "ESP_BLE_AD_TYPE_32SERVICE_DATA";
        case ESP_BLE_AD_TYPE_128SERVICE_DATA: return "ESP_BLE_AD_TYPE_128SERVICE_DATA";
        case ESP_BLE_AD_TYPE_LE_SECURE_CONFIRM: return "ESP_BLE_AD_TYPE_LE_SECURE_CONFIRM";
        case ESP_BLE_AD_TYPE_LE_SECURE_RANDOM: return "ESP_BLE_AD_TYPE_LE_SECURE_RANDOM";
        case ESP_BLE_AD_TYPE_URI: return "ESP_BLE_AD_TYPE_URI";
        case ESP_BLE_AD_TYPE_INDOOR_POSITION: return "ESP_BLE_AD_TYPE_INDOOR_POSITION";
        case ESP_BLE_AD_TYPE_TRANS_DISC_DATA: return "ESP_BLE_AD_TYPE_TRANS_DISC_DATA";
        case ESP_BLE_AD_TYPE_LE_SUPPORT_FEATURE: return "ESP_BLE_AD_TYPE_LE_SUPPORT_FEATURE";
        case ESP_BLE_AD_TYPE_CHAN_MAP_UPDATE: return "ESP_BLE_AD_TYPE_CHAN_MAP_UPDATE";
        case ESP_BLE_AD_MANUFACTURER_SPECIFIC_TYPE: return "ESP_BLE_AD_MANUFACTURER_SPECIFIC_TYPE";
        default: return "UNKNOWN_TYPE";
    }
}


const char* get_ble_appearance_name(uint16_t code) {
    switch (code) {
        case 0x0000: return "Unknown";
        case 0x0040: return "Generic Phone";
        case 0x0080: return "Generic Computer";
        case 0x00C0: return "Generic Watch";
        case 0x00C1: return "Sports Watch";
        case 0x0100: return "Generic Clock";
        case 0x0140: return "Generic Display";
        case 0x0180: return "Generic Remote";
        case 0x01C0: return "Generic Eyeglasses";
        case 0x0200: return "Generic Tag";
        case 0x0240: return "Generic Keyring";
        case 0x0280: return "Generic Media Player";
        case 0x02C0: return "Generic Barcode Scanner";
        case 0x0300: return "Generic Thermometer";
        case 0x0301: return "Thermometer (Ear)";
        case 0x0340: return "Generic Heart Rate";
        case 0x0341: return "Heart Rate Belt";
        case 0x0380: return "Generic Blood Pressure";
        case 0x0381: return "Blood Pressure (Arm)";
        case 0x0382: return "Blood Pressure (Wrist)";
        case 0x03C0: return "Generic HID";
        case 0x03C1: return "HID Keyboard";
        case 0x03C2: return "HID Mouse";
        case 0x03C3: return "HID Joystick";
        case 0x03C4: return "HID Gamepad";
        case 0x03C5: return "HID Digitizer Tablet";
        case 0x03C6: return "HID Card Reader";
        case 0x03C7: return "HID Digital Pen";
        case 0x03C8: return "HID Barcode Scanner";
        case 0x0400: return "Generic Glucose Meter";
        case 0x0440: return "Generic Walking Sensor";
        case 0x0441: return "Walking Sensor (In-Shoe)";
        case 0x0442: return "Walking Sensor (On-Shoe)";
        case 0x0443: return "Walking Sensor (On-Hip)";
        case 0x0480: return "Generic Cycling";
        case 0x0481: return "Cycling Computer";
        case 0x0482: return "Cycling Speed Sensor";
        case 0x0483: return "Cycling Cadence Sensor";
        case 0x0484: return "Cycling Power Sensor";
        case 0x0485: return "Cycling Speed and Cadence Sensor";
        case 0x0841: return "Standalone Speaker";
        case 0x0C40: return "Generic Pulse Oximeter";
        case 0x0C41: return "Pulse Oximeter (Fingertip)";
        case 0x0C42: return "Pulse Oximeter (Wrist)";
        case 0x0C80: return "Generic Weight Scale";
        case 0x0CC0: return "Generic Personal Mobility Device";
        case 0x0CC1: return "Powered Wheelchair";
        case 0x0CC2: return "Mobility Scooter";
        case 0x0D00: return "Generic Continuous Glucose Monitor";
        case 0x0D40: return "Generic Insulin Pump";
        case 0x0D41: return "Durable Insulin Pump";
        case 0x0D44: return "Patch Insulin Pump";
        case 0x0D48: return "Insulin Pen";
        case 0x0D80: return "Generic Medication Delivery Device";
        case 0x1440: return "Generic Outdoor Sports Sensor";
        case 0x1441: return "Outdoor Sports Location Sensor";
        case 0x1442: return "Outdoor Sports Location and Navigation";
        case 0x1443: return "Outdoor Sports Location Pod";
        case 0x1444: return "Outdoor Sports Location Pod and Navigation";
        default: return "Unknown Code";
    }
}


#ifdef __cplusplus
}
#endif

#endif /* __ESP_GAP_BLE_API_H__ */
