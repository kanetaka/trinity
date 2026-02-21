cmake_minimum_required(VERSION 3.16)

project(no9 LANGUAGES CXX)

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
include_directories(${CMAKE_SOURCE_DIR})

# Library directories
link_directories(${VK_SDK_PATH}/Lib)

# Header files (for IDE organization)
set(HEADERS
    triangle_app.h
    no9_app.h
    core/vulkan_context.h
    core/command_buffer.h
    core/image_barrier.h
    core/swapchain.h
    core/surface_provider.h
    core/sdl2_surface_provider.h
)

# Source files
set(SOURCES
    main.cpp
    triangle_app.cpp
    core/vulkan_context.cpp
    core/command_buffer.cpp
    core/image_barrier.cpp
    core/swapchain.cpp
    core/sdl2_surface_provider.cpp
)

# Create executable
add_executable(no9 ${HEADERS} ${SOURCES})

# Link libraries
target_link_libraries(no9
    SDL2main
    SDL2
    vulkan-1
)

# Set this as a GUI application (equivalent to gui_app : true in meson)
if(WIN32)
    set_target_properties(no9 PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
    
    # Fix runtime library conflicts and suppress PDB warnings
    target_link_options(no9 PRIVATE 
        $<$<CONFIG:Debug>:/NODEFAULTLIB:msvcrt.lib>
        $<$<CONFIG:Release>:/NODEFAULTLIB:msvcrtd.lib>
        /ignore:4099  # Suppress PDB warnings for external libraries
    )
endif()

# Set output directory (optional)
set_target_properties(no9 PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
