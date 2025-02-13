# This file was automatically generated for projects
# without default 'CMakeLists.txt' file.

FILE(GLOB_RECURSE app_sources ${CMAKE_SOURCE_DIR}/src/*.*)

# This compiles but does not create itself. To create it, use board_build.embed_txtfiles in platformio.ini.
idf_component_register(SRCS ${app_sources}
    EMBED_TXTFILES
        "www/index.html"
        "www/css.css"
    EMBED_FILES
        "www/index.html.gz"
        "www/css.css.gz"
)

#EMBED_TXTFILES
#"www/index.html"
#"www/css.css"

#idf_component_register(SRCS "web.cpp"
#                        EMBED_FILES "www/index.html" "web/css.css")
#idf_component_register(SRCS "modules/web/web.cpp"
#                        EMBED_TXTFILES "www/index.html" "web/css.css")
