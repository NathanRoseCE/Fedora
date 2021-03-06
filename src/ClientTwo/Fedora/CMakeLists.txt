cmake_minimum_required(VERSION 3.13)

project(Fedora)

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
file(GLOB Fedora_src
     "src/*.cpp"
)
add_library(${PROJECT_NAME} ${Fedora_src})
target_include_directories(${PROJECT_NAME} PUBLIC include)
target_include_directories(${PROJECT_NAME} PRIVATE src/include)
target_link_libraries(${PROJECT_NAME} PRIVATE microxrcedds_agent microxrcedds_client $<$<C_COMPILER_ID:GNU>:-Wl,--gc-section,--no-export-dynamic>)

if(MSVC OR MSVC_IDE)
  target_compile_options(${PROJECT_NAME} PRIVATE /wd4996)
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES
  C_STANDARD 99
  C_STANDARD_REQUIRED YES
  )

