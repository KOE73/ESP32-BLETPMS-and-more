cmake_minimum_required(VERSION 3.16.0)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(Auto_BLETPMS_ELM327)

# For LVGL
if(CONFIG_FREERTOS_SMP)
    include_directories(${IDF_PATH}/components/freertos/FreeRTOS-Kernel-SMP/include/freertos/)
    #target_include_directories(${COMPONENT_LIB} PRIVATE "${IDF_PATH}/components/freertos/FreeRTOS-Kernel-SMP/include/freertos/")
else()
    include_directories(${IDF_PATH}/components/freertos/FreeRTOS-Kernel/include/freertos/)
    #target_include_directories(${COMPONENT_LIB} PRIVATE "${IDF_PATH}/components/freertos/FreeRTOS-Kernel/include/freertos/")
endif()


#set(EXTRA_COMPONENT_DIRS $ENV{IDF_PATH}/examples/common_components/arduino)

## Укажите стандарт C++
#set(CMAKE_CXX_STANDARD 17)          # Устанавливает стандарт C++17
#set(CMAKE_CXX_STANDARD_REQUIRED ON) # Обязательно использовать указанный стандарт
#set(CMAKE_CXX_EXTENSIONS OFF)       # Отключить нестандартные расширения компилятора
