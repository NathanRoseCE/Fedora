cmake_minimum_required(VERSION 2.8.12)
if (${CMAKE_VERSION} VERSION_GREATER 3.0)
  cmake_policy(SET CMP0048 NEW)
endif()

project(Broker)

find_package(microxrcedds_client REQUIRED)
find_package(microxrcedds_agent REQUIRED)

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

set(DDS_XRCE_Message_Source_location "/usr/src/Fedora/messages/")
file(GLOB DDS_XRCE_MessagerSources "${DDS_XRCE_Message_Source_location}/*.c")
set(Message_include_dir ${DDS_XRCE_Message_Source_location})

add_executable(${PROJECT_NAME} src/ClientThree.cpp src/Broker.cpp ${DDS_XRCE_MessagerSources})
target_compile_options(${PROJECT_NAME} PRIVATE "-DUCLIENT_MAX_OUTPUT_RELIABLE_STREAMS=5")
target_include_directories(${PROJECT_NAME} PRIVATE ${Message_include_dir})
if(MSVC OR MSVC_IDE)
  target_compile_options(${PROJECT_NAME} PRIVATE /wd4996)
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES
  C_STANDARD 99
  C_STANDARD_REQUIRED YES
  )

target_link_libraries(${PROJECT_NAME} microxrcedds_agent microxrcedds_client $<$<C_COMPILER_ID:GNU>:-Wl,--gc-section,--no-export-dynamic>)

