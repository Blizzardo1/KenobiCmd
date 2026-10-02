cmake_minimum_required(VERSION 3.31)
project(kenobicmd C)

find_package(PkgConfig REQUIRED)
pkg_check_modules(HIDAPI REQUIRED hidapi-hidraw)

add_executable(kenobicmd main.c pretty.c arguments.c)

add_compile_options(-fsanitize=address -Wall -Wextra -Wpedantic -fstack-usage)

target_include_directories(kenobicmd PRIVATE ${HIDAPI_INCLUDE_DIRS})
target_link_libraries(kenobicmd PRIVATE ${HIDAPI_LIBRARIES})
target_compile_options(kenobicmd PRIVATE ${HIDAPI_CFLAGS_OTHER})