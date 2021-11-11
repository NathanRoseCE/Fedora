cmake_minimum_required(VERSION 3.13)

project(Fedora)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/609281088cfefc76f9d0ce82e1ff6c30cc3591e5.zip
  )
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)
include(GoogleTest)

find_package(microxrcedds_client REQUIRED)
find_package(microxrcedds_agent REQUIRED)
add_subdirectory(libs)

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
file(GLOB Fedora_src
     "src/BrokerImpl.cpp"
     "src/Broker.cpp"
)
add_library(Fedora ${Fedora_src})
target_include_directories(${PROJECT_NAME} PUBLIC include)
target_include_directories(${PROJECT_NAME} PRIVATE src/include)
target_link_libraries(${PROJECT_NAME} PRIVATE microxrcedds_agent microxrcedds_client $<$<C_COMPILER_ID:GNU>:-Wl,--gc-section,--no-export-dynamic>)
target_link_libraries(${PROJECT_NAME} PUBLIC rapid_xml)

if(MSVC OR MSVC_IDE)
  target_compile_options(${PROJECT_NAME} PRIVATE /wd4996)
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES
  C_STANDARD 99
  C_STANDARD_REQUIRED YES
  )


add_executable(
  TestBroker
  "testing/TestBroker.cpp"
  )
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/resources/config.xml
  DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/resources)

target_link_libraries(
  TestBroker PUBLIC gtest_main Fedora
  )
gtest_discover_tests(TestBroker)

