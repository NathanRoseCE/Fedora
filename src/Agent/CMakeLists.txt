cmake_minimum_required(VERSION 3.12.4)

if(NOT CMAKE_VERSION VERSION_LESS 3.0)
    cmake_policy(SET CMP0048 NEW)
endif()

project(FedoraLogger)

list(APPEND CMAKE_MODULE_PATH /usr/src/dds/install)
# Find requirements
if(NOT fastcdr_FOUND)
    find_package(fastcdr REQUIRED)
endif()

if(NOT fastrtps_FOUND)
    find_package(fastrtps REQUIRED)
endif()

# Set C++11
include(CheckCXXCompilerFlag)
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_CLANG OR
        CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    check_cxx_compiler_flag(-std=c++11 SUPPORTS_CXX11)
    if(SUPPORTS_CXX11)
        add_compile_options(-std=c++11)
    else()
        message(FATAL_ERROR "Compiler doesn't support C++11")
    endif()
endif()
set(DDS_Message_Source_location "/usr/src/Fedora/messages/")

message(STATUS "Adding Message sources...")
file(GLOB messageSources "${DDS_Message_Source_location}/*.cxx")
set(Message_include_dir messages)

add_executable(BasicLogger src/Logger.cpp ${messageSources})
target_include_directories(BasicLogger PRIVATE ${Message_include_dir})
target_link_libraries(BasicLogger fastrtps fastcdr)

