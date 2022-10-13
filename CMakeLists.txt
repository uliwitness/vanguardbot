cmake_minimum_required(VERSION 3.23)
project(linuxtestproject)

set(CMAKE_CXX_STANDARD 23)

add_executable(vanguardbot unix/main.cpp
        unix/vanguardbot_base.cpp
        unix/vanguardbot_base.hpp
        common/ini_file.cpp
        common/ini_file.hpp
        common/string_utils.cpp
        common/string_utils.hpp
        common/vanguardbot.cpp
        common/vanguardbot.hpp)

target_include_directories(vanguardbot PRIVATE unix)
target_include_directories(vanguardbot PRIVATE common)
