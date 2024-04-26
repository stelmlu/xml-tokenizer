cmake_minimum_required(VERSION 3.28.2)
project(test_xml_tokenizer VERSION 1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/example)

# Add executable
add_executable(${PROJECT_NAME} main.cpp example/read_catalog.c)

# Copy the xml file to the build directory
configure_file(${CMAKE_SOURCE_DIR}/book_catalog.xml ${CMAKE_BINARY_DIR}/book_catalog.xml COPYONLY)
