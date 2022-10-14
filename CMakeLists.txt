cmake_minimum_required(VERSION 3.23)
project(vanguardbot)

set(CMAKE_CXX_STANDARD 23)

add_executable(vanguardbot unix/main.cpp
        unix/vanguardbot_base.cpp
        unix/vanguardbot_base.hpp
        common/ini_file.cpp
        common/ini_file.hpp
        common/string_utils.cpp
        common/string_utils.hpp
        common/vanguardbot.cpp
        common/vanguardbot.hpp
        common/client_id.h
        common/twitch.cpp
        common/twitch.hpp
        json11/json11.cpp
        json11/json11.hpp)

find_package( CURL REQUIRED )

target_include_directories(vanguardbot PRIVATE unix)
target_include_directories(vanguardbot PRIVATE common)
target_include_directories(vanguardbot SYSTEM PRIVATE ${CURL_INCLUDE_DIRS})

target_link_libraries(vanguardbot CURL::libcurl)

