cmake_minimum_required(VERSION 3.16)

# Auto-detect vcpkg toolchain if not specified or invalid
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE OR NOT EXISTS "${CMAKE_TOOLCHAIN_FILE}")
    if(DEFINED ENV{VCPKG_ROOT} AND EXISTS "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
        set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "" FORCE)
    else()
        find_program(VCPKG_EXECUTABLE vcpkg)
        if(VCPKG_EXECUTABLE)
            get_filename_component(VCPKG_DIR "${VCPKG_EXECUTABLE}" DIRECTORY)
            if(EXISTS "${VCPKG_DIR}/scripts/buildsystems/vcpkg.cmake")
                set(CMAKE_TOOLCHAIN_FILE "${VCPKG_DIR}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "" FORCE)
            endif()
        endif()
    endif()
endif()

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

# Find vcpkg packages
find_package(CLI11 CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)

# --- Library: tri_core ---
add_library(tri_core STATIC
    src/core/asset_path.h
    src/core/asset_path.cpp
)
target_include_directories(tri_core PUBLIC src)
target_link_libraries(tri_core PUBLIC Vulkan::Vulkan glm::glm)

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
    src/graphics/graphics_context.h
    src/graphics/graphics_context.cpp
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
    src/scene/component/i_pre_renderable.h
    src/scene/component/splat_component.h
    src/scene/component/splat_component.cpp
    src/scene/component/point_cloud_component.h
    src/scene/component/point_cloud_component.cpp
    src/scene/io/ply_loader.h
    src/scene/io/ply_loader.cpp
    src/scene/io/shader_loader.h
    src/scene/io/shader_loader.cpp
    src/scene/io/splat_types.h
    src/scene/io/point_cloud_types.h
)

target_include_directories(tri_scene PUBLIC src)
target_link_libraries(tri_scene PUBLIC tri_core tri_geometry tri_graphics Vulkan::Vulkan)

# --- Library: tri_renderer ---
add_library(tri_renderer STATIC
    src/renderer/renderer.h
    src/renderer/renderer.cpp
    src/renderer/ui/ui_manager.h
    src/renderer/ui/ui_manager.cpp
)
target_include_directories(tri_renderer PUBLIC src)
target_link_libraries(tri_renderer PUBLIC tri_core tri_graphics tri_scene Vulkan::Vulkan SDL3::SDL3 imgui::imgui)

# --- Library: tri_app ---
add_library(tri_app STATIC
    src/app/application.h
    src/app/application.cpp
    src/app/i_application.h
)
target_compile_definitions(tri_app PUBLIC GLM_FORCE_DEPTH_ZERO_TO_ONE)
target_include_directories(tri_app PUBLIC src)
target_link_libraries(tri_app
    PUBLIC
    tri_core
    tri_geometry
    tri_graphics
    tri_scene
    tri_renderer
    SDL3::SDL3
    Vulkan::Vulkan
    nlohmann_json::nlohmann_json
)

# --- Executable: trinity ---
add_executable(trinity
    src/main.cpp
)
target_include_directories(trinity PRIVATE src)
target_link_libraries(trinity
    PRIVATE
    tri_app
    CLI11::CLI11
    nlohmann_json::nlohmann_json
)

# Compile shaders with Slang
find_program(SLANGC_EXECUTABLE NAMES slangc PATHS ${VK_SDK_PATH}/Bin)
if(SLANGC_EXECUTABLE)
    file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/asset/shader/splat)
    file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/asset/shader/point_cloud)

    set(SLANG_FLAGS -target spirv -matrix-layout-column-major -fvk-use-gl-layout)

    add_custom_command(
        OUTPUT ${CMAKE_SOURCE_DIR}/asset/shader/splat/splat.vert.spv
        COMMAND ${SLANGC_EXECUTABLE} ${CMAKE_SOURCE_DIR}/asset/shader/splat/splat.slang
                ${SLANG_FLAGS} -entry vsMain -stage vertex
                -o ${CMAKE_SOURCE_DIR}/asset/shader/splat/splat.vert.spv
        DEPENDS ${CMAKE_SOURCE_DIR}/asset/shader/splat/splat.slang
        COMMENT "Compiling 3DGS vertex shader with Slang..."
    )
    add_custom_command(
        OUTPUT ${CMAKE_SOURCE_DIR}/asset/shader/splat/splat.frag.spv
        COMMAND ${SLANGC_EXECUTABLE} ${CMAKE_SOURCE_DIR}/asset/shader/splat/splat.slang
                ${SLANG_FLAGS} -entry fsMain -stage fragment
                -o ${CMAKE_SOURCE_DIR}/asset/shader/splat/splat.frag.spv
        DEPENDS ${CMAKE_SOURCE_DIR}/asset/shader/splat/splat.slang
        COMMENT "Compiling 3DGS fragment shader with Slang..."
    )
    add_custom_command(
        OUTPUT ${CMAKE_SOURCE_DIR}/asset/shader/point_cloud/point_cloud.vert.spv
        COMMAND ${SLANGC_EXECUTABLE} ${CMAKE_SOURCE_DIR}/asset/shader/point_cloud/point_cloud.slang
                ${SLANG_FLAGS} -entry vsMain -stage vertex
                -o ${CMAKE_SOURCE_DIR}/asset/shader/point_cloud/point_cloud.vert.spv
        DEPENDS ${CMAKE_SOURCE_DIR}/asset/shader/point_cloud/point_cloud.slang
        COMMENT "Compiling PointCloud vertex shader with Slang..."
    )
    add_custom_command(
        OUTPUT ${CMAKE_SOURCE_DIR}/asset/shader/point_cloud/point_cloud.frag.spv
        COMMAND ${SLANGC_EXECUTABLE} ${CMAKE_SOURCE_DIR}/asset/shader/point_cloud/point_cloud.slang
                ${SLANG_FLAGS} -entry fsMain -stage fragment
                -o ${CMAKE_SOURCE_DIR}/asset/shader/point_cloud/point_cloud.frag.spv
        DEPENDS ${CMAKE_SOURCE_DIR}/asset/shader/point_cloud/point_cloud.slang
        COMMENT "Compiling PointCloud fragment shader with Slang..."
    )
    add_custom_target(compile_shaders ALL DEPENDS 
        ${CMAKE_SOURCE_DIR}/asset/shader/splat/splat.vert.spv 
        ${CMAKE_SOURCE_DIR}/asset/shader/splat/splat.frag.spv
        ${CMAKE_SOURCE_DIR}/asset/shader/point_cloud/point_cloud.vert.spv
        ${CMAKE_SOURCE_DIR}/asset/shader/point_cloud/point_cloud.frag.spv
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

# Visual Studio debugger settings
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT trinity)
set_target_properties(trinity PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    VS_DEBUGGER_ENVIRONMENT "PATH=${CMAKE_BINARY_DIR}/bin/$(Configuration);${CMAKE_BINARY_DIR}/vcpkg_installed/x64-windows/$<IF:$<CONFIG:Debug>,debug/bin,bin>;${CMAKE_BINARY_DIR}/vcpkg_installed/x64-windows/bin;$ENV{VK_SDK_PATH}/Bin;%PATH%"
)

# Copy runtime DLLs to output directory on post-build
add_custom_command(TARGET trinity POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
        "${CMAKE_BINARY_DIR}/vcpkg_installed/x64-windows/$<IF:$<CONFIG:Debug>,debug/bin,bin>"
        "$<TARGET_FILE_DIR:trinity>"
)

