project('x8', 'cpp', default_options : ['cpp_std=c++20'])
compiler = meson.get_compiler('cpp')

deps = [
    compiler.find_library('SDL2', dirs : 'C:/VulkanSDK/1.4.313.2/Lib'),
    compiler.find_library('SDL2main', dirs : 'C:/VulkanSDK/1.4.313.2/Lib'),
    compiler.find_library('vulkan-1', dirs : 'C:/VulkanSDK/1.4.313.2/Lib')
]

executable('x8_app', 'main.cpp',
    'vulkan_extern.h', 'vulkan_extern.cpp',
    'vulkan_function.h', 'vulkan_function.cpp',
    include_directories : include_directories('C:/VulkanSDK/1.4.313.2/Include'),
    dependencies : deps,
    gui_app : true
)
