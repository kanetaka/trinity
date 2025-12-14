cmake_minimum_required(VERSION 3.16)

# Project configuration
set(PROJECT_NAME x8)
project(${PROJECT_NAME} LANGUAGES CXX)

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

# Header files organized by category
set(ROOT_HEADERS
    triangle_app.h
)

set(COMMON_HEADERS
    common/application.h
)

set(CORE_HEADERS
    core/buffer_resource.h
    core/command_buffer.h
    core/gpu_resource_base.h
    core/image_barrier.h
    core/sdl2_surface_provider.h
    core/surface_provider.h
    core/swapchain.h
    core/vertex_buffer.h
    core/vulkan_context.h
)

# Source files organized by category
set(ROOT_SOURCES
    main.cpp
    triangle_app.cpp
)

set(CORE_SOURCES
    core/buffer_resource.cpp
    core/command_buffer.cpp
    core/image_barrier.cpp
    core/sdl2_surface_provider.cpp
    core/swapchain.cpp
    core/vertex_buffer.cpp
    core/vulkan_context.cpp
)

# Combine all sources and headers
set(ALL_HEADERS ${ROOT_HEADERS} ${COMMON_HEADERS} ${CORE_HEADERS})
set(ALL_SOURCES ${ROOT_SOURCES} ${CORE_SOURCES})

# Create executable
add_executable(${PROJECT_NAME} ${ALL_HEADERS} ${ALL_SOURCES})

# Create Visual Studio source filters (virtual folders)
source_group("Header Files" FILES ${ROOT_HEADERS})
source_group("Header Files\\common" FILES ${COMMON_HEADERS})
source_group("Header Files\\core" FILES ${CORE_HEADERS})

source_group("Source Files" FILES ${ROOT_SOURCES})
source_group("Source Files\\core" FILES ${CORE_SOURCES})

# Link libraries
target_link_libraries(${PROJECT_NAME}
    SDL2main
    SDL2
    vulkan-1
)

# Set this as a GUI application (equivalent to gui_app : true in meson)
if(WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
    
    # Fix runtime library conflicts and suppress PDB warnings
    target_link_options(${PROJECT_NAME} PRIVATE 
        $<$<CONFIG:Debug>:/NODEFAULTLIB:msvcrt.lib>
        $<$<CONFIG:Release>:/NODEFAULTLIB:msvcrtd.lib>
        /ignore:4099  # Suppress PDB warnings for external libraries
    )
endif()

# Set output directory (optional)
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
