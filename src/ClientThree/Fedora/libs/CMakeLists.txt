#add_subdirectory(rapidxml-1.13)
add_library(rapid_xml INTERFACE)
target_include_directories(rapid_xml INTERFACE rapidxml-1.13/ )
