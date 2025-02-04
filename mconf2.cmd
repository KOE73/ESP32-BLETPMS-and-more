:set SDKCONFIG=custom_sdkconfig
pio run -t menuconfig -- --config "sdkconfig.ESP32-C6-ST7789-SD-debug"
:pio run -t menuconfig -D CONFIG_FILE=my_config.conf.
