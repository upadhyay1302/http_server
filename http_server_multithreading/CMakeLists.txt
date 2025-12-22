# ------------------------------------------------------------
# CMake minimum version
# ------------------------------------------------------------
cmake_minimum_required(VERSION 3.10)

# ------------------------------------------------------------
# Project definition
# ------------------------------------------------------------
project(server VERSION 1.0 LANGUAGES CXX)

# ------------------------------------------------------------
# Source files
# ------------------------------------------------------------
set(SERVER_SOURCES
    driver.cpp
    request.cpp
    SocketUtils.cpp
    WorkerPool.cpp
    ThreadSafeCout.cpp
)

# ------------------------------------------------------------
# Executable target
# ------------------------------------------------------------
add_executable(${PROJECT_NAME} ${SERVER_SOURCES})

# ------------------------------------------------------------
# C++ standard (target-based, modern CMake)
# ------------------------------------------------------------
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_11)

# ------------------------------------------------------------
# Include directories
# ------------------------------------------------------------
target_include_directories(${PROJECT_NAME}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# ------------------------------------------------------------
# Compiler warnings (optional but recommended)
# ------------------------------------------------------------
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
elseif (MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4)
endif()

# ------------------------------------------------------------
# Build information
# ------------------------------------------------------------
message(STATUS "Project Name: ${PROJECT_NAME}")
message(STATUS "Project Version: ${PROJECT_VERSION}")
message(STATUS "Source Directory: ${PROJECT_SOURCE_DIR}")
message(STATUS "Binary Directory: ${PROJECT_BINARY_DIR}")
