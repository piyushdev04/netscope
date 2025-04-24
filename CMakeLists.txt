# Minimum version of CMake required
cmake_minimum_required(VERSION 3.10)

# Project name
project(NetworkingTools)

# Set C++ standard to C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories
include_directories(backend)  # Add backend as an include directory

# Add executable with all source files
add_executable(networking_tools
    backend/main.cpp
    backend/handlers.cpp
    backend/port_scanner.cpp
    backend/ping_tool.cpp        # Add ping_tool.cpp
    backend/whois_lookup.cpp     # Add whois_lookup.cpp
    backend/ip_info.cpp          # Add ip_info.cpp
)

# Link libraries
target_link_libraries(networking_tools
    pthread        # For multi-threading support
    curl           # For any curl-related networking
    stdc++fs       # Link with filesystem if needed (for GCC < 9.1)
)

# Optional: Set the output directory for the executable
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)