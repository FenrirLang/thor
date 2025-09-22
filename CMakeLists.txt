cmake_minimum_required(VERSION 3.20)
project(Thor)

# C++ Standard #
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Output directories #
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Create include directory #
file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Gather source files #
file(GLOB_RECURSE SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

set(TARGET_NAME "thor")
add_executable(${TARGET_NAME} ${SOURCES})

target_include_directories(${TARGET_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include/)

# Link against filesystem library for C++17 filesystem support
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS "9.0")
    target_link_libraries(${TARGET_NAME} stdc++fs)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS "9.0")
    target_link_libraries(${TARGET_NAME} stdc++fs)
endif()