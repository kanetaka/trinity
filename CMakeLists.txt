cmake_minimum_required(VERSION 3.16)

project(kousoku LANGUAGES CXX)

# Set C++20 standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set runtime library to avoid conflicts (global setting)
if(WIN32)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
endif()

# Vulkan SDK path from environment variable
set(VK_SDK_PATH $ENV{VK_SDK_PATH})

# Check if VK_SDK_PATH is set
if(NOT VK_SDK_PATH)
    message(FATAL_ERROR "VK_SDK_PATH environment variable is not set. Please set it to your Vulkan SDK installation path.")
endif()

# Include directories
include_directories(${VK_SDK_PATH}/Include)

# Library directories
link_directories(${VK_SDK_PATH}/Lib)

# Source files
set(SOURCES
    main.cpp
    vulkan_context.cpp
)

# Header files (for IDE organization)
set(HEADERS
    vulkan_context.h
    triangle_app.h
    kousoku_app.h
    core/command_buffer.h
    core/swapchain.h
)

# Create executable
add_executable(kousoku ${SOURCES} ${HEADERS})

# Link libraries
target_link_libraries(kousoku
    SDL2main
    SDL2
    vulkan-1
)

# Set this as a GUI application (equivalent to gui_app : true in meson)
if(WIN32)
    set_target_properties(kousoku PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
    
    # Fix runtime library conflicts and suppress PDB warnings
    target_link_options(kousoku PRIVATE 
        $<$<CONFIG:Debug>:/NODEFAULTLIB:msvcrt.lib>
        $<$<CONFIG:Release>:/NODEFAULTLIB:msvcrtd.lib>
        /ignore:4099  # Suppress PDB warnings for external libraries
    )
endif()

# Set output directory (optional)
set_target_properties(kousoku PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
