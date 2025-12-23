# ------------------------------------------------------------
# CMake configuration
# ------------------------------------------------------------
cmake_minimum_required(VERSION 3.10)

project(http_server
    VERSION 1.0
    DESCRIPTION "Epoll-based HTTP server in C++"
    LANGUAGES CXX
)

# ------------------------------------------------------------
# Compiler settings
# ------------------------------------------------------------
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Enable warnings (good practice for systems projects)
if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# ------------------------------------------------------------
# Source files
# ------------------------------------------------------------
set(SRC_FILES
    driver.cpp
    request.cpp
    socket_utils.cpp
)

# ------------------------------------------------------------
# Build target
# ------------------------------------------------------------
add_executable(${PROJECT_NAME} ${SRC_FILES})

target_include_directories(${PROJECT_NAME}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# ------------------------------------------------------------
# Build info
# ------------------------------------------------------------
message(STATUS "Building project: ${PROJECT_NAME}")
message(STATUS "C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Source dir: ${PROJECT_SOURCE_DIR}")
message(STATUS "Build dir: ${PROJECT_BINARY_DIR}")
