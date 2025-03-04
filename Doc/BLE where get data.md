
Public YAML files for Bluetooth SIG Assigned Numbers, GATT Specification Supplement, and Device Properties
https://bitbucket.org/bluetooth-SIG/public/src/main/assigned_numbers/uuids/

About HR 
https://bitbucket.org/bluetooth-SIG/public/src/main/gss/org.bluetooth.characteristic.heart_rate_measurement.yaml

TPMS
https://www.instructables.com/BLE-Direct-Tire-Pressure-Monitoring-System-TPMS-Di/
https://bitbucket.org/bluetooth-SIG/public/src/main/assigned_numbers/company_identifiers/company_identifiers.yaml find 0100 'TomTom International BV'

```
TPMS BLE "manufacturer data" format
"000180eaca108a78e36d0000e60a00005b00"
 0001                                    Manufacturer (0001: TomTom)
     80                                  Sensor Number (80:1, 81:2, 82:3, 83:4, ..)
     80eaca108a78                        Sensor Address
                 e36d0000                Pressure
                         e60a0000        Temperature
                                 5b      Battery percentage
                                   00    Alarm Flag (00: OK, 01: No Pressure Alarm)
```