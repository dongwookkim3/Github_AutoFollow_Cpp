cmake_minimum_required(VERSION 3.30)
project(Github_AutoFollow_Cpp)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_TOOLCHAIN_FILE /path/to/vcpkg/scripts/buildsystems/vcpkg.cmake)

add_executable(Github_AutoFollow_Cpp main.cpp)

find_package(CURL REQUIRED)
target_link_libraries(Github_AutoFollow_Cpp PRIVATE CURL::libcurl)