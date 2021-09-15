cmake_minimum_required(VERSION 2.8.12)
if (${CMAKE_VERSION} VERSION_GREATER 3.0)
  cmake_policy(SET CMP0048 NEW)
endif()

project(PubClient)

find_package(microxrcedds_client REQUIRED)

set(DDS_XRCE_Message_Source_location "/usr/src/Fedora/messages/")
file(GLOB DDS_XRCE_MessagerSources "${DDS_XRCE_Message_Source_location}/*.c")
set(Message_include_dir ${DDS_XRCE_Message_Source_location})
add_executable(${PROJECT_NAME} src/PubClient.cpp ${DDS_XRCE_MessagerSources})
target_include_directories(${PROJECT_NAME} PRIVATE ${Message_include_dir})
if(MSVC OR MSVC_IDE)
  target_compile_options(${PROJECT_NAME} PRIVATE /wd4996)
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES
  C_STANDARD 99
  C_STANDARD_REQUIRED YES
  )

target_link_libraries(${PROJECT_NAME} microxrcedds_client $<$<C_COMPILER_ID:GNU>:-Wl,--gc-section,--no-export-dynamic>)

