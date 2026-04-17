cmake_minimum_required(VERSION 3.16)

project(trinity LANGUAGES CXX)

# Set C++20 standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    add_compile_options(/utf-8)
endif()

# Set runtime library to avoid conflicts (global setting)
if(WIN32)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
endif()

# Find Vulkan
find_package(Vulkan REQUIRED)

# Find SDL3
set(VK_SDK_PATH $ENV{VK_SDK_PATH})
if(VK_SDK_PATH)
    list(APPEND CMAKE_PREFIX_PATH ${VK_SDK_PATH}/cmake ${VK_SDK_PATH})
endif()
find_package(SDL3 REQUIRED)

# --- Library: tri_core ---
add_library(tri_core STATIC
    src/core/asset_path.h
    src/core/asset_path.cpp
    src/core/registry.h
    src/core/components.h
    src/core/systems.h
    src/core/systems.cpp
)
target_include_directories(tri_core PUBLIC src)
# Core uses Vulkan SDK for GLM etc
target_link_libraries(tri_core PUBLIC Vulkan::Vulkan)

# --- Library: tri_geo ---
add_library(tri_geo STATIC
    src/geo/camera.h
    src/geo/camera.cpp
)
target_include_directories(tri_geo PUBLIC src)
target_link_libraries(tri_geo PUBLIC tri_core SDL3::SDL3)

# --- Library: tri_io ---
add_library(tri_io STATIC
    src/io/ply_loader.h
    src/io/ply_loader.cpp
    src/io/shader_loader.h
    src/io/shader_loader.cpp
    src/io/splat_types.h
)
target_include_directories(tri_io PUBLIC src)
target_link_libraries(tri_io PUBLIC tri_core Vulkan::Vulkan)

# --- Library: tri_render ---
add_library(tri_render STATIC
    src/render/command_buffer.h
    src/render/command_buffer.cpp
    src/render/extension_features.h
    src/render/image_barrier.h
    src/render/image_barrier.cpp
    src/render/renderer.h
    src/render/renderer.cpp
    src/render/swapchain.h
    src/render/swapchain.cpp
    src/render/vulkan_context.h
    src/render/vulkan_context.cpp
    src/render/pipeline/graphics_pipeline.h
    src/render/pipeline/graphics_pipeline_builder.h
    src/render/pipeline/graphics_pipeline_builder.cpp
    src/render/resource/buffer_resource.h
    src/render/resource/buffer_resource.cpp
    src/render/resource/gpu_resource_base.h
    src/render/resource/image_resource.h
    src/render/resource/image_resource.cpp
    src/render/resource/sampler.h
    src/render/resource/sampler.cpp
    src/render/surface/surface_provider.h
    src/render/surface/sdl3_surface_provider.h
    src/render/surface/sdl3_surface_provider.cpp
    src/render/component/splat_data_component.h
    src/render/system/splat_system.h
    src/render/system/splat_system.cpp
)
target_include_directories(tri_render PUBLIC src)
target_link_libraries(tri_render PUBLIC tri_core tri_geo tri_io Vulkan::Vulkan SDL3::SDL3)

# --- Library: tri_ui ---
add_library(tri_ui STATIC
    src/ui/ui_manager.h
    src/ui/ui_manager.cpp
    3rd/trimgui/imgui.cpp
    3rd/trimgui/imgui_demo.cpp
    3rd/trimgui/imgui_draw.cpp
    3rd/trimgui/imgui_tables.cpp
    3rd/trimgui/imgui_widgets.cpp
    3rd/trimgui/backends/imgui_impl_sdl3.cpp
    3rd/trimgui/backends/imgui_impl_vulkan.cpp
)
target_include_directories(tri_ui PUBLIC src 3rd/trimgui 3rd/trimgui/backends)
target_link_libraries(tri_ui PUBLIC tri_core tri_render Vulkan::Vulkan SDL3::SDL3)

# --- Executable: trinity ---
add_executable(trinity
    src/main.cpp
    src/app/application.h
    src/app/application.cpp
    src/app/common/trinity_app.h
)
target_compile_definitions(trinity PRIVATE GLM_FORCE_DEPTH_ZERO_TO_ONE)
target_include_directories(trinity PRIVATE src)
target_link_libraries(trinity
    PRIVATE
    tri_core
    tri_geo
    tri_io
    tri_render
    tri_ui
    SDL3::SDL3
    Vulkan::Vulkan
)

# Compile shaders
find_program(GLSLC_EXECUTABLE NAMES glslc PATHS ${VK_SDK_PATH}/Bin)
if(GLSLC_EXECUTABLE)
    file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/asset/shader/splat)
    add_custom_command(
        OUTPUT ${CMAKE_SOURCE_DIR}/asset/shader/splat/splat.vert.spv
               ${CMAKE_SOURCE_DIR}/asset/shader/splat/splat.frag.spv
        COMMAND ${GLSLC_EXECUTABLE} ${CMAKE_SOURCE_DIR}/asset/shader/splat/splat.vert -o ${CMAKE_SOURCE_DIR}/asset/shader/splat/splat.vert.spv
        COMMAND ${GLSLC_EXECUTABLE} ${CMAKE_SOURCE_DIR}/asset/shader/splat/splat.frag -o ${CMAKE_SOURCE_DIR}/asset/shader/splat/splat.frag.spv
        DEPENDS ${CMAKE_SOURCE_DIR}/asset/shader/splat/splat.vert ${CMAKE_SOURCE_DIR}/asset/shader/splat/splat.frag
        COMMENT "Compiling 3DGS shaders..."
    )
    add_custom_target(compile_shaders ALL DEPENDS 
        ${CMAKE_SOURCE_DIR}/asset/shader/splat/splat.vert.spv 
        ${CMAKE_SOURCE_DIR}/asset/shader/splat/splat.frag.spv
    )
    add_dependencies(trinity compile_shaders)
endif()

# Platform specific settings
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

# Set output directory
set_target_properties(trinity PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
