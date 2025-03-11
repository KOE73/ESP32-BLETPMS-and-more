# This file was automatically generated for projects
# without default 'CMakeLists.txt' file.

# message(STATUS "++++++++++++++++++++++++++++++++++STATUS: check out STATUS")
# message(WARNING "++++++++++++++++++++++++++++++++++Warning: check out WARNING")
# message(SEND_ERROR "++++++++++++++++++++++++++++++++++Error: check out SEND_ERROR")
# message(FATAL_ERROR "++++++++++++++++++++++++++++++++++Critical: check out FATAL_ERROR")

FILE(GLOB_RECURSE app_sources ${CMAKE_SOURCE_DIR}/src/*.*)

# Ищем все заголовочные файлы в папке src
FILE(GLOB_RECURSE MY_HEADER_DIRS "${CMAKE_SOURCE_DIR}/src/*")

# message(SEND_ERROR "++++++++++++++++++++++++++++++++++")
# message(SEND_ERROR "++++++++++++++++++++++++++++++++++ IDF_PATH ${IDF_PATH}")
# message(SEND_ERROR "++++++++++++++++++++++++++++++++++ ESP_PLATFORM ${ESP_PLATFORM} ")
# message(SEND_ERROR "++++++++++++++++++++++++++++++++++ CONFIG_FREERTOS_SMP ${CONFIG_FREERTOS_SMP}")
# message(SEND_ERROR "++++++++++++++++++++++++++++++++++ MY_HEADER_DIRS ${MY_HEADER_DIRS}")


#if(CONFIG_FREERTOS_SMP)
#    target_include_directories(${COMPONENT_LIB} PRIVATE "${IDF_PATH}/components/freertos/FreeRTOS-Kernel-SMP/include/freertos/")
#else()
#    #message("${COMPONENT_LIB} PRIVATE ${IDF_PATH}/components/freertos/FreeRTOS-Kernel/include/freertos/")
#    target_include_directories(${COMPONENT_LIB} PRIVATE "${IDF_PATH}/components/freertos/FreeRTOS-Kernel/include/freertos/")
#endif()



# Указываем компилятору, где искать файлы
#include_directories(${MY_HEADER_DIRS}/src/*.*)

# This compiles but does not create itself. To create it, use board_build.embed_txtfiles in platformio.ini.
idf_component_register(SRCS ${app_sources}
    #INCLUDE_DIRS
        #${CMAKE_SOURCE_DIR}/src/modules/web_server
        #${MY_HEADER_DIRS}
    EMBED_TXTFILES
        "www/index.html"
        "www/css.css"
    EMBED_FILES
        "www/.gz/ble.html.gz"
        "www/.gz/index.html.gz"
        "www/.gz/css.css.gz"
)



#EMBED_TXTFILES
#"www/index.html"
#"www/css.css"

#idf_component_register(SRCS "web.cpp"
#                        EMBED_FILES "www/index.html" "web/css.css")
#idf_component_register(SRCS "modules/web/web.cpp"
#                        EMBED_TXTFILES "www/index.html" "web/css.css")
