# This file was automatically generated for projects
# without default 'CMakeLists.txt' file.

FILE(GLOB_RECURSE app_sources ${CMAKE_SOURCE_DIR}/src/*.*)

# Ищем все заголовочные файлы в папке src
FILE(GLOB_RECURSE MY_HEADER_DIRS "${CMAKE_SOURCE_DIR}/src/*")

# Указываем компилятору, где искать файлы
#include_directories(${MY_HEADER_DIRS}/src/*.*)

# This compiles but does not create itself. To create it, use board_build.embed_txtfiles in platformio.ini.
idf_component_register(SRCS ${app_sources}
#    INCLUDE_DIRS
#        ${CMAKE_SOURCE_DIR}/src/modules/web_server
        #${MY_HEADER_DIRS}
    EMBED_TXTFILES
        "www/index.html"
        "www/css.css"
    EMBED_FILES
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
