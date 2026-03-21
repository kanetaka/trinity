cmake_minimum_required(VERSION 3.16)

project(trinity LANGUAGES CXX)

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

# Add Vulkan SDK to the prefix path so find_package can locate SDL3
# Note: Newer Vulkan SDKs (e.g. 1.4.341.1) place SDL3Config.cmake in the 'cmake' subdirectory,
# while older versions (e.g. 1.4.328.1) may place it in 'Lib/cmake/SDL3'.
# We add both to ensure the correct one is found.
list(APPEND CMAKE_PREFIX_PATH ${VK_SDK_PATH}/cmake ${VK_SDK_PATH})

# Find SDL3
find_package(SDL3 REQUIRED)

# Include directories
include_directories(${VK_SDK_PATH}/Include)
include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${CMAKE_SOURCE_DIR}/src/app)
include_directories(${CMAKE_SOURCE_DIR}/src/app/common)
include_directories(${CMAKE_SOURCE_DIR}/src/core)
include_directories(${CMAKE_SOURCE_DIR}/src/render)
include_directories(${CMAKE_SOURCE_DIR}/src/render/pipeline)
include_directories(${CMAKE_SOURCE_DIR}/src/render/resources)
include_directories(${CMAKE_SOURCE_DIR}/src/render/surface)
include_directories(${CMAKE_SOURCE_DIR}/src/core/ecs)
include_directories(${CMAKE_SOURCE_DIR}/src/core)
include_directories(${CMAKE_SOURCE_DIR}/src/render)
include_directories(${CMAKE_SOURCE_DIR}/src/components)
include_directories(${CMAKE_SOURCE_DIR}/src/utils)

# Library directories
link_directories(${VK_SDK_PATH}/Lib)

# Header files (for IDE organization)
file(GLOB_RECURSE HEADERS "src/*.h")

# Source files
file(GLOB_RECURSE SOURCES "src/*.cpp")

# Create executable
add_executable(trinity ${HEADERS} ${SOURCES})
target_compile_definitions(trinity PRIVATE GLM_FORCE_DEPTH_ZERO_TO_ONE)

# Link libraries
target_link_libraries(trinity
    PRIVATE
    vulkan-1
    SDL3::SDL3
)

# Compile shaders
find_program(GLSLC_EXECUTABLE NAMES glslc PATHS ${VK_SDK_PATH}/Bin)
if(GLSLC_EXECUTABLE)
    file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/assets)
    add_custom_command(
        OUTPUT ${CMAKE_SOURCE_DIR}/assets/shaders/splat/splat.vert.spv
               ${CMAKE_SOURCE_DIR}/assets/shaders/splat/splat.frag.spv
        COMMAND ${GLSLC_EXECUTABLE} ${CMAKE_SOURCE_DIR}/assets/shaders/splat/splat.vert -o ${CMAKE_SOURCE_DIR}/assets/shaders/splat/splat.vert.spv
        COMMAND ${GLSLC_EXECUTABLE} ${CMAKE_SOURCE_DIR}/assets/shaders/splat/splat.frag -o ${CMAKE_SOURCE_DIR}/assets/shaders/splat/splat.frag.spv
        DEPENDS ${CMAKE_SOURCE_DIR}/assets/shaders/splat/splat.vert ${CMAKE_SOURCE_DIR}/assets/shaders/splat/splat.frag
        COMMENT "Compiling 3DGS shaders..."
    )
    add_custom_target(compile_shaders ALL DEPENDS 
        ${CMAKE_SOURCE_DIR}/assets/shaders/splat/splat.vert.spv 
        ${CMAKE_SOURCE_DIR}/assets/shaders/splat/splat.frag.spv
    )
    add_dependencies(trinity compile_shaders)
endif()

# Set this as a GUI application (equivalent to gui_app : true in meson)
if(WIN32)
    set_target_properties(trinity PROPERTIES
        WIN32_EXECUTABLE FALSE
    )
    
    # Fix runtime library conflicts and suppress PDB warnings
    target_link_options(trinity PRIVATE 
        $<$<CONFIG:Debug>:/NODEFAULTLIB:msvcrt.lib>
        $<$<CONFIG:Release>:/NODEFAULTLIB:msvcrtd.lib>
        /ignore:4099  # Suppress PDB warnings for external libraries
    )
endif()

# Set output directory (optional)
set_target_properties(trinity PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
