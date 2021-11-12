cmake_minimum_required(VERSION 3.10)
enable_testing()
project(ClientThree)
add_subdirectory(Fedora)

set(DDS_XRCE_Message_Source_location "/usr/src/Fedora/messages/")
file(GLOB DDS_XRCE_MessagerSources "${DDS_XRCE_Message_Source_location}/*.c")
set(Message_include_dir ${DDS_XRCE_Message_Source_location})

add_executable(${PROJECT_NAME} src/ClientThree.cpp ${DDS_XRCE_MessagerSources})
target_compile_options(${PROJECT_NAME} PRIVATE "-DUCLIENT_MAX_OUTPUT_RELIABLE_STREAMS=5")
target_include_directories(${PROJECT_NAME} PRIVATE ${Message_include_dir})
if(MSVC OR MSVC_IDE)
  target_compile_options(${PROJECT_NAME} PRIVATE /wd4996)
endif()

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/resources/config.xml
  DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/resources)
set_target_properties(${PROJECT_NAME} PROPERTIES
  C_STANDARD 99
  C_STANDARD_REQUIRED YES
  )

target_link_libraries(${PROJECT_NAME} PRIVATE Fedora)
