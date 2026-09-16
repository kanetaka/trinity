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
)
target_include_directories(tri_core PUBLIC src)
# Core uses Vulkan SDK for GLM etc
target_link_libraries(tri_core PUBLIC Vulkan::Vulkan)

# --- Library: tri_geometry ---
add_library(tri_geometry STATIC
    src/geometry/camera.h
    src/geometry/camera.cpp
)
target_include_directories(tri_geometry PUBLIC src)
target_link_libraries(tri_geometry PUBLIC tri_core SDL3::SDL3)

# --- Library: tri_graphics ---
add_library(tri_graphics STATIC
    src/graphics/command_buffer.h
    src/graphics/command_buffer.cpp
    src/graphics/extension_features.h
    src/graphics/image_barrier.h
    src/graphics/image_barrier.cpp
    src/graphics/swapchain.h
    src/graphics/swapchain.cpp
    src/graphics/vulkan_context.h
    src/graphics/vulkan_context.cpp
    src/graphics/pipeline/graphics_pipeline.h
    src/graphics/pipeline/graphics_pipeline_builder.h
    src/graphics/pipeline/graphics_pipeline_builder.cpp
    src/graphics/resource/buffer_resource.h
    src/graphics/resource/buffer_resource.cpp
    src/graphics/resource/gpu_resource_base.h
    src/graphics/resource/image_resource.h
    src/graphics/resource/image_resource.cpp
    src/graphics/resource/sampler.h
    src/graphics/resource/sampler.cpp
    src/graphics/surface/surface_provider.h
    src/graphics/surface/sdl3_surface_provider.h
    src/graphics/surface/sdl3_surface_provider.cpp
)
target_include_directories(tri_graphics PUBLIC src)
target_link_libraries(tri_graphics PUBLIC tri_core Vulkan::Vulkan SDL3::SDL3)

# --- Library: tri_scene ---
add_library(tri_scene STATIC
    src/scene/component.h
    src/scene/object.h
    src/scene/object.cpp
    src/scene/scene.h
    src/scene/scene.cpp
    src/scene/component/i_renderable.h
    src/scene/component/splat_component.h
    src/scene/component/splat_component.cpp
    src/scene/io/ply_loader.h
    src/scene/io/ply_loader.cpp
    src/scene/io/shader_loader.h
    src/scene/io/shader_loader.cpp
    src/scene/io/splat_types.h
)
target_include_directories(tri_scene PUBLIC src)
target_link_libraries(tri_scene PUBLIC tri_core tri_geometry tri_graphics Vulkan::Vulkan)

# --- Library: tri_renderer ---
add_library(tri_renderer STATIC
    src/renderer/renderer.h
    src/renderer/renderer.cpp
)
target_include_directories(tri_renderer PUBLIC src)
target_link_libraries(tri_renderer PUBLIC tri_core tri_graphics tri_scene Vulkan::Vulkan)

# --- Library: tri_ui ---
add_library(tri_ui STATIC
    src/ui/ui_manager.h
    src/ui/ui_manager.cpp
    lib/imgui/imgui.cpp
    lib/imgui/imgui_demo.cpp
    lib/imgui/imgui_draw.cpp
    lib/imgui/imgui_tables.cpp
    lib/imgui/imgui_widgets.cpp
    lib/imgui/backends/imgui_impl_sdl3.cpp
    lib/imgui/backends/imgui_impl_vulkan.cpp
)
target_include_directories(tri_ui PUBLIC src lib/imgui lib/imgui/backends)
target_link_libraries(tri_ui PUBLIC tri_core tri_graphics Vulkan::Vulkan SDL3::SDL3)

# --- Library: tri_app ---
add_library(tri_app STATIC
    src/app/application.h
    src/app/application.cpp
    src/app/iapplication.hpp
)
target_compile_definitions(tri_app PUBLIC GLM_FORCE_DEPTH_ZERO_TO_ONE)
target_include_directories(tri_app PUBLIC src lib/json/include)
target_link_libraries(tri_app
    PUBLIC
    tri_core
    tri_geometry
    tri_graphics
    tri_scene
    tri_renderer
    tri_ui
    SDL3::SDL3
    Vulkan::Vulkan
)

# --- Executable: trinity ---
add_executable(trinity
    src/main.cpp
)
target_include_directories(trinity PRIVATE src lib/CLI11/include lib/json/include)
target_link_libraries(trinity
    PRIVATE
    tri_app
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
