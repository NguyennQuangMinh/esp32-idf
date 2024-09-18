cmake_minimum_required(VERSION 3.5)

set(EXTRA_COMPONENT_DIRS
    $ENV{IDF_PATH}/examples/common_components/led_strip
    ${CMAKE_CURRENT_LIST_DIR}/common
)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

project(blink_video_2)
